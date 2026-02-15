#ifdef ENABLE_POLYPHONY

#include <math.h>
#include "Voice.h"
#include "Cetone033.h"

CVoice::CVoice(void)
{
	this->Oscs[0] = new CSynthOscillator();
	this->Oscs[1] = new CSynthOscillator();
	this->Filter = new CCetoneLpFilter();

	this->Active = false;
	this->Age = 0;
	this->CurrentNote = -1;
	this->CurrentVelocity = 0;
	this->CurrentPitch = 0;

	this->VelocityMod = 0.f;
	this->VelocityModStep = 0.f;
	this->VelocityModEnd = 0.f;

	this->VoiceVolume[0] = 0.f;
	this->VoiceVolume[1] = 0.f;

	this->EnvPos[0] = -1;
	this->EnvPos[1] = -1;

	this->DecayResonance = 0.f;

	this->DoGlide = false;
	this->GlidePitch = 0;
	this->GlideStep = 0;
	this->GlideFrac = 0;
	this->GlideSamples = 0.f;
}

CVoice::~CVoice(void)
{
	delete this->Oscs[0];
	delete this->Oscs[1];
	delete this->Filter;
}

void CVoice::NoteOn(int note, int velocity, float sampleRate, float modChangeSamples,
                    int coarse0, int fine0, int coarse1, int fine1,
                    float morph0, float morph1, int wave0, int wave1,
                    float resonance, bool glideState, float glideSpeed,
                    int lastPitch, bool hasLastPitch)
{
	int  tmp;
	bool glide = (glideState && (glideSpeed != 0.f) && hasLastPitch) ? true : false;

	this->Active = true;
	this->Age = 0;
	this->CurrentNote = note;
	this->CurrentVelocity = velocity;
	this->VelocityModEnd = (float)velocity / 127.f;

	if (this->VelocityModEnd != this->VelocityMod) {
		this->VelocityModStep = (this->VelocityModEnd - this->VelocityMod) * modChangeSamples;
	} else
		this->VelocityModStep = 0.f;

	tmp = (note + NOTE_OFFSET) * 100;

	if (glide) {
		this->GlideSamples = floorf(glideSpeed * sampleRate + 0.5f);
		this->GlideStep = (int)(((tmp - lastPitch) / (this->GlideSamples)) * 16384.f + 0.5f);
		this->GlideFrac = lastPitch << 14;
		this->GlidePitch = tmp;
		this->CurrentPitch = lastPitch;
	} else {
		this->CurrentPitch = tmp;
	}

	this->Oscs[0]->Set(morph0, wave0);
	this->Oscs[1]->Set(morph1, wave1);

	this->EnvPos[0] = 0;
	this->EnvPos[1] = 0;

	this->DecayResonance = resonance;

	this->DoGlide = glide;
}

void CVoice::NoteOff()
{
	// For now, immediately deactivate the voice
	// In a full implementation, you might want to trigger a release envelope
	this->Active = false;
	this->EnvPos[0] = -1;
	this->EnvPos[1] = -1;
	this->VoiceVolume[0] = 0.f;
	this->VoiceVolume[1] = 0.f;
}

float CVoice::Process(float evolume0, float evolume1, float evolume2,
                      int pitch0, int pitch1,
                      float attackFactor0, float decayFactor0,
                      float attackFactor1, float decayFactor1,
                      float modEnv, float modVel, float sampleRateEnv, float sampleRateVel,
                      float cutoff, float resonance, float modResValue,
                      int filterCounter, bool clipState)
{
	if (!this->Active)
		return 0.f;

	float output = 0.f;

	// Velocity mod slide
	if (this->VelocityModStep != 0.f) {
		this->VelocityMod += this->VelocityModStep;

		if (this->VelocityModStep < 0.f) {
			if (this->VelocityMod <= this->VelocityModEnd) {
				this->VelocityMod = this->VelocityModEnd;
				this->VelocityModStep = 0.f;
			}
		} else {
			if (this->VelocityMod >= this->VelocityModEnd) {
				this->VelocityMod = this->VelocityModEnd;
				this->VelocityModStep = 0.f;
			}
		}
	}

	// Glide
	if (this->DoGlide) {
		this->GlideFrac += this->GlideStep;
		int itmp = this->GlideFrac >> 14;

		if (this->GlideStep < 0) {
			if (itmp <= this->GlidePitch) {
				itmp = this->GlidePitch;
				this->DoGlide = false;
			}
		} else {
			if (itmp >= this->GlidePitch) {
				itmp = this->GlidePitch;
				this->DoGlide = false;
			}
		}

		this->CurrentPitch = itmp;
	}

	this->Oscs[0]->SetPitch(this->CurrentPitch + pitch0);
	this->Oscs[1]->SetPitch(this->CurrentPitch + pitch1);

	// Envelopes
	for (int i = 0; i < 2; i++) {
		switch (this->EnvPos[i]) {
		case 0:
			this->VoiceVolume[i] += (i == 0 ? attackFactor0 : attackFactor1);
			if (this->VoiceVolume[i] >= 1.f) {
				this->VoiceVolume[i] = 1.f;
				this->EnvPos[i] = 1;
			}
			break;
		case 1:
			this->VoiceVolume[i] -= (i == 0 ? decayFactor0 : decayFactor1);
			if (this->VoiceVolume[i] <= 0.f) {
				this->VoiceVolume[i] = 0.f;
				this->EnvPos[i] = -1;
				// If both envelopes finished, deactivate voice
				if (this->EnvPos[0] == -1 && this->EnvPos[1] == -1) {
					this->Active = false;
				}
			}
			break;
		default:
			this->VoiceVolume[i] = 0.f;
			break;
		}
	}

	float output_volume = this->VoiceVolume[0] * evolume0;

	// Oscillators
	float f;

	f = this->Oscs[0]->Run() * evolume1;
	if (f > 2.f)
		f = 2.f;
	else if (f < -2.f)
		f = -2.f;

	output += f;

	f = this->Oscs[1]->Run() * evolume2;

	if (f > 2.f)
		f = 2.f;
	else if (f < -2.f)
		f = -2.f;

	output = output + f;

	// Filters
	if (filterCounter == FILTER_DELAY) {
		float c = cutoff;
		c += modEnv * this->VoiceVolume[1] * sampleRateEnv;
		c += modVel * this->VelocityMod * sampleRateVel;
		this->DecayResonance *= modResValue;
		float r = resonance * this->DecayResonance;
		this->Filter->Set(c, r);
	}

	output = this->Filter->Run(output);

	// End
	output *= output_volume;

	if (clipState) {
		float s;

		if (output < 0.f) {
			s = -1.f;
			output = -output;
		} else
			s = 1.f;

		if (output > 1.f) {
			output = 0.9f;
		} else if (output >= 0.8f) {
			output = 0.8f + (output - 0.8f) / (1.f + powf(((output - 0.8f) * 5.f), 2.f));
		}

		output *= s;
	}

	return output;
}

void CVoice::SetFilterSampleRate(float fs)
{
	this->Filter->SetSampleRate(fs);
}

void CVoice::SetFilterType(int type)
{
	this->Filter->SetType(type);
}

void CVoice::SetFilterParams(float cutoff, float resonance)
{
	this->Filter->Set(cutoff, resonance);
}

void CVoice::ResetFilter()
{
	this->Filter->Reset();
}

float CVoice::WarmupFilter()
{
	return this->Filter->Run(0.f);
}

#endif // ENABLE_POLYPHONY
