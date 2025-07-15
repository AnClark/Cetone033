#pragma once

// DPF-specific preset structure.
//
// Each parameter conforms to VST 2.4 standard (param range is [0.0f, 1.0f]),
// for easily interacting with AnClark's plugin implementation, which uses the same range.

#include "extra/String.hpp"
#include "structures.h"

class CCetoneUI;

struct SynthPreset_DPF {
    DISTRHO::String Name;
    float           Parameter[pParameters];
};

class PresetManager {
    CCetoneUI* ui;

public:
    PresetManager(CCetoneUI* ui)
        : ui(ui)
    {
    }

    void            applyPreset(const SynthPreset_DPF& preset);
    SynthPreset_DPF dumpPreset(const char* name);

    SynthPreset_DPF pickFactoryPreset(uint32_t index);
    void            applyFactoryPreset(uint32_t index);

private:
    float _getDefaultParamValue(uint8_t index);
    float _sanityCheck(uint8_t index, float paramValue);
};
