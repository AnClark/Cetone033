#include "PresetManager.hpp"
#include "CetoneUI.hpp"
#include "defines.h"

#include "PresetData_DPF.hpp"

void PresetManager::applyPreset(const SynthPreset_DPF& preset)
{
    // TODO: Store preset name in plugin state
    for (uint8_t i = 0; i < pParameters; i++)
    {
        float new_value = _sanityCheck(i, preset.Parameter[i]);
        ui->setParameterValue(i, new_value);    // Apply new preset value
        ui->parameterChanged(i, new_value);     // Refresh UI param value
    }
}

SynthPreset_DPF PresetManager::dumpPreset(const char* name)
{
    SynthPreset_DPF p;

    p.Name = name;

    p.Parameter[pVolume] = ui->fKnobVolume->getValue();

    p.Parameter[pOsc1Coarse] = ui->fKnobOsc1Coarse->getValue();
    p.Parameter[pOsc1Fine] = ui->fKnobOsc1Fine->getValue();
    p.Parameter[pOsc1Wave] = ui->fKnobOsc1Waveform->getValue();
    p.Parameter[pOsc1Morph] = ui->fKnobOsc1Morph->getValue();
    p.Parameter[pOsc1Volume] = ui->fKnobOsc1Volume->getValue();

    p.Parameter[pOsc2Coarse] = ui->fKnobOsc2Coarse->getValue();
    p.Parameter[pOsc2Fine] = ui->fKnobOsc2Fine->getValue();
    p.Parameter[pOsc2Wave] = ui->fKnobOsc2Waveform->getValue();
    p.Parameter[pOsc2Morph] = ui->fKnobOsc2Morph->getValue();
    p.Parameter[pOsc2Volume] = ui->fKnobOsc2Volume->getValue();

    p.Parameter[pGlideSpeed] = ui->fKnobGlideSpeed->getValue();

    p.Parameter[pEnv1Attack] = ui->fAmpAttack->getValue();
    p.Parameter[pEnv1Decay] = ui->fAmpDecay->getValue();

    p.Parameter[pEnv2Attack] = ui->fModAttack->getValue();
    p.Parameter[pEnv2Decay] = ui->fModDecay->getValue();

    p.Parameter[pModEnv] = ui->fModEnvelope->getValue();
    p.Parameter[pModVel] = ui->fModVelocity->getValue();
    p.Parameter[pModRes] = ui->fModResDecay->getValue();

    p.Parameter[pCutoff] = ui->fFilterCutoff->getValue();
    p.Parameter[pResonance] = ui->fFilterResonance->getValue();
    p.Parameter[pFilterType] = ui->fFilterType->getValue();

    p.Parameter[pGlideState] = ui->fBtnGlideState->isDown() ? 1.0f : 0.0f;
    p.Parameter[pClipState] = ui->fBtnClipState->isDown() ? 1.0f : 0.0f;

    return p;
}

SynthPreset_DPF PresetManager::pickFactoryPreset(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index >= 0 && index < FactoryPreset_Count, SynthPreset_DPF())

    return FactoryPresets[index];
}

void PresetManager::applyFactoryPreset(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index >= 0 && index < FactoryPreset_Count, )

    applyPreset(pickFactoryPreset(index));
}

float PresetManager::_getDefaultParamValue(uint8_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index >= 0 && index < pParameters, 0.0f)

    // NOTICE: Volume algorithm here is different from CetoneSynthLight.
    constexpr float DEFAULT_VOLUME_PARAM_VALUE = 0.5f;

    // Param default values are extracted from CCetonUI::CCetoneUI() (widgets' default values).
    switch (index) {
    case pVolume:
        return DEFAULT_VOLUME_PARAM_VALUE;

    case pOsc1Coarse:
        return 0.5f;
    case pOsc1Fine:
        return 0.5f;
    case pOsc1Wave:
        return ui->_pi2f(WAVE_SAW, WAVE_MAX);
    case pOsc1Morph:
        return 0.5f;
    case pOsc1Volume:
        return DEFAULT_VOLUME_PARAM_VALUE;

    case pOsc2Coarse:
        return 0.38f;
    case pOsc2Fine:
        return 0.5f;
    case pOsc2Wave:
        return ui->_pi2f(WAVE_PULSE, WAVE_MAX);
    case pOsc2Morph:
        return 0.5f;
    case pOsc2Volume:
        return DEFAULT_VOLUME_PARAM_VALUE;

    case pGlideSpeed:
        return 0.0f;

    case pEnv1Attack:
        return 0.02f;
    case pEnv1Decay:
        return 0.40f;

    case pEnv2Attack:
        return 0.02f;
    case pEnv2Decay:
        return 0.15f;

    case pModEnv:
        return 0.7f;
    case pModVel:
        return 0.0f;
    case pModRes:
        return 0.0f;

    case pCutoff:
        return 1.0f;
    case pResonance:
        return 0.0f;
    case pFilterType:
        return 0.0f;

    // Clip and Glide are OFF by default.
    case pClipState:
    case pGlideState:
        return 0.0f;

    default:
        return 0.0f;
    }
}

float PresetManager::_sanityCheck(uint8_t index, float paramValue)
{
    // Check if param value legal. If not, fallback to default.
    
    DISTRHO_SAFE_ASSERT_RETURN(index >= 0 && index < pParameters, 0.0f)

    return (paramValue >= 0.0f && paramValue <= 1.0f) ? paramValue : _getDefaultParamValue(index);
}
