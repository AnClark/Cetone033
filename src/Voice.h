#pragma once

#ifdef ENABLE_POLYPHONY

#include "SynthOscillator.h"
#include "CetoneLpFilter.h"

class CVoice
{
public:
	CVoice(void);
	~CVoice(void);

	// Voice management
	bool IsActive() const { return Active; }
	void SetActive(bool active) { Active = active; }
	int GetNote() const { return CurrentNote; }
	int GetAge() const { return Age; }
	void IncrementAge() { Age++; }
	void ResetAge() { Age = 0; }

	// Note control
	void NoteOn(int note, int velocity, float sampleRate, float modChangeSamples,
	            int coarse0, int fine0, int coarse1, int fine1,
	            float morph0, float morph1, int wave0, int wave1,
	            float resonance, bool glideState, float glideSpeed, 
	            int lastPitch, bool hasLastPitch);
	void NoteOff();

	// Audio processing
	float Process(float evolume0, float evolume1, float evolume2,
	              int pitch0, int pitch1,
	              float attackFactor0, float decayFactor0,
	              float attackFactor1, float decayFactor1,
	              float modEnv, float modVel, float sampleRateEnv, float sampleRateVel,
	              float cutoff, float resonance, float modResValue,
	              int filterCounter, bool clipState);

	// Getters
	int GetCurrentPitch() const { return CurrentPitch; }

	// Filter control
	void SetFilterSampleRate(float fs);
	void SetFilterType(int type);
	void SetFilterParams(float cutoff, float resonance);
	void ResetFilter();
	float WarmupFilter();  // For filter warm-up during initialization

private:
	CSynthOscillator* Oscs[2];
	CCetoneLpFilter*  Filter;

	bool  Active;
	int   Age;
	
	int   CurrentNote;
	int   CurrentVelocity;
	int   CurrentPitch;

	float VelocityMod;
	float VelocityModStep;
	float VelocityModEnd;

	float VoiceVolume[2];
	int   EnvPos[2];
	float DecayResonance;

	bool  DoGlide;
	int   GlidePitch;
	int   GlideStep;
	int   GlideFrac;
	float GlideSamples;
};

#endif // ENABLE_POLYPHONY
