#pragma once

#include <cstdint>
#include <vector>

#include "structures.h"

// Forward decls.
class CCetoneUI;

class CetonePresetManager
{
    // AnClark's new preset format
    std::vector<SynthPreset_DPF>   FactoryPresets;

    // Old official storages of program, using internal param values.
    SynthProgram      FactoryPrograms[128];
    SynthProgramOld   OldPrograms[128];

public:
    CetonePresetManager();

    // -------------------------------------------------------------------
    // APIs for AnClark's preset formats

    void LoadFactoryPresets_DPF(const SynthPreset_DPF* bank, uint32_t count);

    void ApplyPreset_DPF(CCetoneUI* ui, const SynthPreset_DPF& preset);

    // -------------------------------------------------------------------
    // APIs for original (old) SynthProgram objects  

    // Load factory programs from a SynthProgram array
    void LoadFactoryPrograms(const SynthProgram* bank, uint32_t count);

    // Load factory programs from a serialized char array (for compatibility with original plugin)
    void LoadFactoryPrograms(unsigned char* factoryPresetData);

    void ImportProgram(SynthProgramOld* src, SynthProgram* dest);

    const SynthProgram GetFactoryProgram(uint8_t index);
    void ApplyProgram(CCetoneUI* ui, const SynthProgram& program);
};
