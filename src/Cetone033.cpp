#include <cmath>

#include "cetone033.h"

bool                 TablesBuilt = false;

extern unsigned char PresetData[];

float                CCetone033::SampleRate;
float                CCetone033::SampleRate2;
float                CCetone033::SampleRateVel;
float                CCetone033::SampleRateEnv;
float                CCetone033::Pi;

float                CCetone033::Int2FloatTab[65536];
float                CCetone033::Int2FloatTab2[65536];

float                CCetone033::SmallSineTable[WAVETABLE_LENGTH];
float                CCetone033::FreqTable[PITCH_MAX];
int                  CCetone033::FreqTableInt[PITCH_MAX];

int                  CCetone033::LookupTable[65536];
float                CCetone033::SawTable[NOTE_MAX * WAVETABLE_LENGTH];
float                CCetone033::ParabolaTable[NOTE_MAX * WAVETABLE_LENGTH];
int                  CCetone033::FreqStepInt[PITCH_MAX];
int                  CCetone033::FreqStepFrac[PITCH_MAX];

CCetone033::CCetone033()
    : DISTRHO::Plugin(pParameters, 0, 0) // parameters, programs, states
{
    this->InitFreqTables(44100.f);

    this->MidiStack = new CMidiStack();

    this->Filter = new CCetoneLpFilter();

    this->Oscs[0] = new CSynthOscillator();
    this->Oscs[1] = new CSynthOscillator();

    this->InitParameters();

    // Explicitly set CetoneSynth's sample rate to fit with DAW's config
    setSampleRate(getSampleRate());
}

CCetone033::~CCetone033()
{
    delete this->MidiStack;
    delete this->Filter;
    delete this->Oscs[0];
    delete this->Oscs[1];
}

void CCetone033::InitFreqTables(float fs)
{
    this->SampleRate = fs;
    this->SampleRate2 = fs * 0.5f;
    this->SampleRateEnv = 0.5f;
    this->SampleRateVel = 0.25f;
    this->Pi = 4.f * atanf(1.f);
    this->ModChangeSamples = 1.f / (fs * MOD_CHANGE_SPEED);

    if (TablesBuilt)
        return;

    float pi = this->Pi;
    float halfpi = pi * 0.5f;

    for (int i = 0; i < PITCH_MAX; i++)
        this->FreqTable[i] = BASE_FREQUENCE * powf(2.f, (float)i / 1200.f);

    for (int i = 0; i < WAVETABLE_LENGTH; i++)
        this->SmallSineTable[i] = sinf(2.f * pi * (float)i / WAVETABLE_LENGTHf);

    int harmonicsIndex = 0;
    int lastHarmonics = -1;
    int lookupIndex = 0;

    for (int i = 0; i < NOTE_MAX; i++) {
        int harmonics = (int)(this->SampleRate2 / this->FreqTable[i * 100]);

        if (harmonics != lastHarmonics) {
            float* ptr = &(this->SawTable[harmonicsIndex * WAVETABLE_LENGTH]);

            for (int n = 0; n < WAVETABLE_LENGTH; n++)
                ptr[n] = 0.f;

            float tharm = halfpi / (float)harmonics;

            for (int n = 0; n < harmonics; n++) {
                int   harmonic = n + 1;

                float t = cosf((float)n * tharm);
                t *= t;
                t /= (float)harmonic;

                for (int m = 0; m < WAVETABLE_LENGTH; m++)
                    ptr[m] += t * this->SmallSineTable[(m * harmonic) & WAVETABLE_MASK];
            }
            lastHarmonics = harmonics;

            int tmp = (int)(2.f * this->FreqTable[i * 100]);

            for (int n = lookupIndex; n <= tmp; n++)
                this->LookupTable[n] = harmonicsIndex;

            lookupIndex = tmp + 1;
            harmonicsIndex++;
        }
    }

    for (int i = lookupIndex; i < 65536; i++)
        this->LookupTable[i] = harmonicsIndex - 1;

    float max = 0.0;

    for (int i = 0; i < WAVETABLE_LENGTH; i++) {
        if (fabs(this->SawTable[i]) > max)
            max = fabs(this->SawTable[i]);
    }

    for (int i = 0; i < harmonicsIndex * WAVETABLE_LENGTH; i++)
        this->SawTable[i] /= max;

    harmonicsIndex = 0;
    lastHarmonics = -1;
    float sign;

    float pi3 = (float)(pi * pi / 3.0);

    for (int i = 0; i < NOTE_MAX; i++) {
        int harmonics = (int)(this->SampleRate2 / this->FreqTable[i * 100]);

        if (harmonics != lastHarmonics) {
            float* ptr = &(this->ParabolaTable[harmonicsIndex * WAVETABLE_LENGTH]);

            for (int n = 0; n < WAVETABLE_LENGTH; n++)
                ptr[n] = pi3;

            float tharm = halfpi / (float)harmonics;

            sign = -1.f;
            for (int n = 0; n < harmonics; n++) {
                float t;
                int   harmonic = n + 1;

                t = cosf((float)n * tharm);
                t *= t;
                t /= (float)(harmonic * harmonic);
                t *= 4.f * sign;

                for (int m = 0; m < WAVETABLE_LENGTH; m++)
                    ptr[m] += t * this->SmallSineTable[(m * harmonic + WAVETABLE_LENGTH4) & WAVETABLE_MASK];
                sign = -sign;
            }
            lastHarmonics = harmonics;
            harmonicsIndex++;
        }
    }

    max = 0.0;

    for (int i = 0; i < WAVETABLE_LENGTH; i++) {
        if (fabs(this->ParabolaTable[i]) > max)
            max = fabs(this->ParabolaTable[i]);
    }

    max /= 2.f;

    for (int i = 0; i < harmonicsIndex * WAVETABLE_LENGTH; i++) {
        this->ParabolaTable[i] /= max;
        this->ParabolaTable[i] -= 1.f;
    }

    for (int i = 0; i < PITCH_MAX; i++) {
        float rate = this->FreqTable[i] * WAVETABLE_LENGTHf / this->SampleRate;

        this->FreqStepInt[i] = (int)rate;
        this->FreqStepFrac[i] = (int)((rate - (float)this->FreqStepInt[i]) * 65536.f);

        this->FreqTableInt[i] = (int)(this->FreqTable[i] * 2.f);
    }

    for (int i = 0; i < 65536; i++) {
        this->Int2FloatTab[i] = (float)i / 65536.f;
        this->Int2FloatTab2[i] = (float)(i - 32768) / 32768.f;
    }

    TablesBuilt = true;
}

void CCetone033::InitParameters()
{
    this->MainVolume = 1.f;

    this->Cutoff = 1.f;
    this->Resonance = 0.f;

    this->Coarse[0] = 0;
    this->Fine[0] = 0;
    this->Wave[0] = WAVE_SAW;
    this->Morph[0] = 0.5f;
    this->Volume[0] = 1.f;

    this->Coarse[1] = -12;
    this->Fine[1] = 0;
    this->Wave[1] = WAVE_PULSE;
    this->Morph[1] = 0.5f;
    this->Volume[1] = 1.f;

    this->ModEnv = 0.2f;
    this->ModVel = 0.f;
    this->ModRes = 0.f;

    this->Attack[0] = 0.02f;
    this->Decay[0] = 0.4f;

    this->Attack[1] = 0.02f;
    this->Decay[1] = 0.15f;

    this->GlideState = false;
    this->GlideSpeed = 0.01f;

    this->ClipState = false;

    this->FilterType = FILTER_TYPE_BIQUAD;

    memcpy(this->OldPrograms, PresetData, sizeof(SynthProgramOld) * 128);

    for (int i = 0; i < 128; i++)
        this->ImportProgram(&OldPrograms[i], &Programs[i]);

    this->ReadProgram(0);

    // Runtime

    this->CurrentDelta = 0;
    this->CurrentNote = -1;
    this->FilterCounter = FILTER_DELAY;

    this->VoiceVolume[0] = 0.f;
    this->VoiceVolume[1] = 0.f;

    this->EnvPos[0] = -1;
    this->EnvPos[1] = -1;

    this->VelocityModStep = 0.f;
    this->DecayResonance = 0.f;

    for (int i = 0; i < 50; i++)
        float t = this->Filter->Run(0.f);
}

void CCetone033::ReadProgram(int prg)
{
    this->CurrentProgram = prg;
    SynthProgram* p = &(this->Programs[prg]);

    for (int i = 0; i < 2; i++) {
        this->Coarse[i] = p->Coarse[i];
        this->Fine[i] = p->Fine[i];
        this->Wave[i] = p->Wave[i];
        this->Volume[i] = p->Volume[i];
        this->Morph[i] = p->Morph[i];
        this->Attack[i] = p->Attack[i];
        this->Decay[i] = p->Decay[i];
    }

    this->ModEnv = p->ModEnv;
    this->ModVel = p->ModVel;
    this->ModRes = p->ModRes;

    this->MainVolume = p->MainVolume;
    this->Cutoff = p->Cutoff;
    this->Resonance = p->Resonance;

    this->ClipState = p->ClipState;

    this->GlideState = p->GlideState;
    this->GlideSpeed = p->GlideSpeed;

    this->FilterType = p->FilterType;

    this->SetGlideSpeed(this->GlideSpeed);
    this->SetGlideState(this->GlideState);
    this->SetModRes(this->ModRes);
    this->UpdateEnvelopes();
    this->Filter->SetType(this->FilterType);
}

void CCetone033::WriteProgram(int prg)
{
    SynthProgram* p = &(this->Programs[prg]);

    for (int i = 0; i < 2; i++) {
        p->Coarse[i] = this->Coarse[i];
        p->Fine[i] = this->Fine[i];
        p->Wave[i] = this->Wave[i];
        p->Volume[i] = this->Volume[i];
        p->Morph[i] = this->Morph[i];
        p->Attack[i] = this->Attack[i];
        p->Decay[i] = this->Decay[i];
    }

    p->ModEnv = this->ModEnv;
    p->ModVel = this->ModVel;
    p->ModRes = this->ModRes;

    p->MainVolume = this->MainVolume;
    p->Cutoff = this->Cutoff;
    p->Resonance = this->Resonance;

    p->ClipState = this->ClipState;

    p->GlideState = this->GlideState;
    p->GlideSpeed = this->GlideSpeed;

    p->FilterType = this->FilterType;
}

START_NAMESPACE_DISTRHO

Plugin* createPlugin()
{
    return new CCetone033();
}

END_NAMESPACE_DISTRHO
