#pragma once

#include "DistrhoPlugin.hpp"
#include "VST2.4_Compatibility.hpp"

#include "defines.h"
#include "globalfunctions.h"
#include "MidiStack.h"
#include "structures.h"
#include "SynthOscillator.h"

#include "CetoneLpFilter.h"

class CCetone033 : public DISTRHO::Plugin {
public:
    CCetone033();
    ~CCetone033(void);

    // ----------------------------------------------------------------------------------------------------------------
    // DISTRHO Plugin interface

    // ---------------------------------------
    // Information

    /**
            Get the plugin label.@n
            This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const noexcept override
    {
        return DISTRHO_PLUGIN_NAME;
    }

    /**
               Get an extensive comment/description about the plugin.@n
               Optional, returns nothing by default.
             */
    const char* getDescription() const override
    {
        return "Light-weight multifunctional synthesizer";
    }

    /**
               Get the plugin author/maker.
             */
    const char* getMaker() const noexcept override
    {
        return DISTRHO_PLUGIN_BRAND;
    }

    /**
               Get the plugin license (a single line of text or a URL).@n
               For commercial plugins this should return some short copyright information.
             */
    const char* getLicense() const noexcept override
    {
        return "GPLv3";
    }

    /**
            Get the plugin version, in hexadecimal.
            @see d_version()
            */
    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    /**
               Get the plugin unique Id.@n
               This value is used by LADSPA, DSSI and VST plugin formats.
               @see d_cconst()
             */
    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('c', '0', '3', '3');
    }

    // ---------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // ---------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;
    void  loadProgram(uint32_t index) override;

    // ---------------------------------------
    // Audio/MIDI Processing

    void activate() override;
    void run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) override;

    // ---------------------------------------
    // Callbacks

    void bufferSizeChanged(int newBufferSize);
    void sampleRateChanged(double newSampleRate) override;

    // -------------------------------------------------------------
    // Legacy VST 2.4 Plugin interface
    // DPF will call them when needed.

    virtual void resume();

    // virtual VstInt32                  processEvents(VstEvents* events);
    virtual VstInt32                  processEvents(const DISTRHO::MidiEvent* events, uint32_t eventCount);
    virtual void                      process(float** inputs, float** outputs, VstInt32 sampleFrames);
    virtual void                      processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames);

    [[maybe_unused]] virtual VstInt32 getChunk(void** data, bool isPreset = false);
    [[maybe_unused]] virtual VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset = false);

    virtual void                      setProgram(VstInt32 program);
    virtual VstInt32                  getProgram();
    virtual void                      setProgramName(char* name);
    virtual void                      getProgramName(char* name);
    virtual bool                      getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);

    virtual VstInt32                  getNumMidiInputChannels();
    virtual VstInt32                  getNumMidiOutputChannels();

    virtual void                      setSampleRate(float sampleRate);
    virtual void                      setBlockSize(VstInt32 blockSize);

    virtual VstInt32                  getVendorVersion();
    virtual bool                      getEffectName(char* name);
    virtual bool                      getVendorString(char* text);
    virtual bool                      getProductString(char* text);

    virtual VstInt32                  canDo(char* text);

    virtual void                      setParameter(VstInt32 index, float value);
    virtual float                     getParameter(VstInt32 index) const;
    virtual void                      getParameterLabel(VstInt32 index, char* label);
    virtual void                      getParameterDisplay(VstInt32 index, char* text);
    virtual void                      getParameterName(VstInt32 index, char* text);
    void                              setParameterAutomated(VstInt32 index, float value) { this->setParameterValue(index, value); }

    static float                      SampleRate;
    static float                      SampleRate2;
    static float                      SampleRateEnv;
    static float                      SampleRateVel;
    static float                      Pi;

    static float                      SmallSineTable[WAVETABLE_LENGTH];
    static float                      FreqTable[PITCH_MAX];
    static int                        FreqTableInt[PITCH_MAX];

    static int                        LookupTable[65536];
    static float                      SawTable[NOTE_MAX * WAVETABLE_LENGTH];
    static float                      ParabolaTable[NOTE_MAX * WAVETABLE_LENGTH];
    static int                        FreqStepInt[PITCH_MAX];
    static int                        FreqStepFrac[PITCH_MAX];

    static float                      Int2FloatTab[65536];
    static float                      Int2FloatTab2[65536];

private:
    CMidiStack*       MidiStack;
    CCetoneLpFilter*  Filter;

    CSynthOscillator* Oscs[2];

    SynthProgram      Programs[128];
    SynthProgramOld   OldPrograms[128];

    int               CurrentNote;
    int               CurrentVelocity;
    int               CurrentDelta;
    int               CurrentProgram;
    int               CurrentPitch;

    float             VelocityMod;
    float             VelocityModStep;
    float             VelocityModEnd;

    int               LastP0;
    int               NextP0;
    int               NextP1;
    int               NextP2;

    int               FilterCounter;

    float             GlideSamples;

    // Program

    int   Coarse[2];
    int   Fine[2];
    int   Wave[2];
    float Morph[2];
    float Volume[2];

    float Attack[2];
    float Decay[2];

    float ModEnv;
    float ModVel;
    float ModRes;

    float Cutoff;
    float Resonance;
    int   FilterType;

    bool  GlideState;
    float GlideSpeed;

    bool  ClipState;
    float MainVolume;

    float CutoffDest;
    float CutoffStep;

    float ResonanceDest;
    float ResonanceStep;

    //

    float AttackFactor[2];
    float DecayFactor[2];
    float VoiceVolume[2];
    int   EnvPos[2];
    float DecayResonance;

    bool  DoGlide;
    int   GlidePitch;
    int   GlideStep;
    int   GlideFrac;
    float ModChangeSamples;
    float ModResValue;

    void  ReadProgram(int prg);
    void  WriteProgram(int prg);

    void  SynthProcess(float** inputs, float** outputs, VstInt32 sampleFrames, bool replace);
    void  HandleMidi(int p0, int p1, int p2);

    void  NoteOn(int note, int vel);
    void  NoteOff(int note, int vel);
    void  InitFreqTables(float fs);
    void  InitParameters();
    void  UpdateEnvelopes();
    void  SetGlideSpeed(float speed);
    void  SetGlideState(bool state);
    void  SetModRes(float value);

    void  SetCutoffSave(float value);
    void  SetResonanceSave(float value);

    void  ImportProgram(SynthProgramOld* src, SynthProgram* dest);
};
