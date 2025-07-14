// WARNING!
//
// In CMake dpf_add_plugin(), must put this source file in FILES_UI section,
// and DO NOT put it in FILES_COMMON section.
// Otherwise, program will stuck!

#include "PresetManager.hpp"
#include "defines.h"
#include "CetoneUI.hpp"
#include "globalfunctions.h"

#include <cstring>

CetonePresetManager::CetonePresetManager()
{
}

void CetonePresetManager::ApplyPreset_DPF(CCetoneUI* ui, const SynthPreset_DPF& preset)
{
    for (int index = 0; index < pParameters; index++)
    {
        // FIXME: Does DPF itself have a sanity check?
        ui->setParameterValue(index, preset.Parameter[index]);
    }
}

void CetonePresetManager::LoadFactoryPrograms(const SynthProgram* bank, uint32_t count)
{
    count = (count < 0) ? 0 : count;
    count = (count >= 128) ? 128 : count;

    for (uint32_t index = 0; index < count; index++)
        this->FactoryPrograms[index] = SynthProgram(bank[index]);
}

void CetonePresetManager::LoadFactoryPrograms(unsigned char* factoryPresetData)
{
    if (!factoryPresetData)
        return;

    // Copy raw preset data to our storage
    memcpy(this->OldPrograms, factoryPresetData, sizeof(SynthProgramOld) * 128);

    // Load programs
    for (int i = 0; i < 128; i++)
        this->ImportProgram(&OldPrograms[i], &FactoryPrograms[i]);
}

void CetonePresetManager::ImportProgram(SynthProgramOld* src, SynthProgram* dest)
{
    for (int i = 0; i < 128; i++)
        dest->Name[i] = src->Name[i];

    dest->Attack[0] = src->Attack[0];
    dest->Attack[1] = src->Attack[1];
    dest->Decay[0] = src->Decay[0];
    dest->Decay[1] = src->Decay[1];
    dest->Coarse[0] = src->Coarse[0];
    dest->Coarse[1] = src->Coarse[1];
    dest->Fine[0] = src->Fine[0];
    dest->Fine[1] = src->Fine[1];
    dest->Wave[0] = src->Wave[0];
    dest->Wave[1] = src->Wave[1];
    dest->Volume[0] = src->Volume[0];
    dest->Volume[1] = src->Volume[1];
    dest->Morph[0] = src->Morph[0];
    dest->Morph[1] = src->Morph[1];

    dest->ClipState = src->ClipState;
    dest->MainVolume = src->MainVolume;

    dest->GlideState = src->GlideState;
    dest->GlideSpeed = src->GlideSpeed;

    dest->FilterType = FILTER_TYPE_BIQUAD;
    dest->Cutoff = src->Cutoff;
    dest->Resonance = src->Resonance;

    dest->ModEnv = src->ModEnv;
    dest->ModRes = src->ModRes;
    dest->ModVel = src->ModVel;
}

const SynthProgram CetonePresetManager::GetFactoryProgram(uint8_t index)
{
    if (index < 0 && index >= 128)
        return SynthProgram();
    
    return FactoryPrograms[index];
}

void CetonePresetManager::ApplyProgram(CCetoneUI* ui, const SynthProgram& p)
{

    ui->setParameterValue(pOsc1Coarse, c_coarse2val(p.Coarse[0]));
    ui->setParameterValue(pOsc2Coarse, c_coarse2val(p.Coarse[1]));

    ui->setParameterValue(pOsc1Fine, c_fine2val(p.Fine[0]));
    ui->setParameterValue(pOsc2Fine, c_fine2val(p.Fine[1]));

    ui->setParameterValue(pOsc1Morph, c_fine2val(p.Morph[0]));
    ui->setParameterValue(pOsc2Morph, c_fine2val(p.Morph[1]));

    ui->setParameterValue(pOsc1Wave, pi2f(p.Wave[0], WAVE_MAX));
    ui->setParameterValue(pOsc2Wave, pi2f(p.Wave[1], WAVE_MAX));

    // FIXME: Volume preset seems buggy! Should compare with official edition.
    ui->setParameterValue(pOsc1Volume, p.Volume[0] * 0.5f);
    ui->setParameterValue(pOsc2Volume, p.Volume[1] * 0.5f);    

    ui->setParameterValue(pVolume, p.MainVolume * 0.5f);
    
#if 0

    case pVolume:
        ret = p->MainVolume * 0.5f;
        break;
    case pCutoff:
        ret = p->Cutoff;
        break;
    case pResonance:
        ret = p->Resonance;
        break;
    case pFilterType:
        ret = pi2f(p->FilterType, FILTER_TYPE_MAX);
        break;

    case pEnv1Attack:
        ret = p->Attack[0];
        break;
    case pEnv2Attack:
        ret = p->Attack[1];
        break;

    case pEnv1Decay:
        ret = p->Decay[0];
        break;
    case pEnv2Decay:
        ret = p->Decay[1];
        break;

    case pModEnv:
        ret = p->ModEnv + 0.5f;
        break;
    case pModVel:
        ret = p->ModVel;
        break;
    case pModRes:
        ret = p->ModRes;
        break;

    case pGlideState:
        ret = c_bool2val(p->GlideState);
        break;
    case pGlideSpeed:
        ret = p->GlideSpeed;
        break;
    case pClipState:
        ret = c_bool2val(p->ClipState);
        break;
    }
#endif
}
