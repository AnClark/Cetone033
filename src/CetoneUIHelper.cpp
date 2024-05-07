#include "CetoneUI.hpp"
#include "defines.h"

constexpr float PARAM_MIN_VALUE = 0.0f;
constexpr float PARAM_MAX_VALUE = 1.0f;
constexpr float PARAM_DEFAULT_VALUE = 0.5f;

void CCetoneUI::_createKnob(ScopedPointer<ImageKnob>& knob, uint32_t paramId, uint absolutePosX, uint absolutePosY, float defaultValue, uint rotationAngle)
{
    Image& knob_image = fImgKnob;

    knob = new ImageKnob(this, knob_image, ImageKnob::Vertical);
    knob->setId(paramId);
    knob->setAbsolutePos(absolutePosX, absolutePosY);
    knob->setRange(PARAM_MIN_VALUE, PARAM_MAX_VALUE);
    knob->setDefault(defaultValue);
    knob->setValue(defaultValue);
    knob->setRotationAngle(rotationAngle);
    knob->setCallback(this);
}

void CCetoneUI::_createSlider(ScopedPointer<ImageSlider>& slider, uint32_t paramId, uint startPosX, uint startPosY, uint endPosX, uint endPosY, float step, bool inverted)
{
#if 0
    slider = new ImageSlider(this, fSliderImage);
    slider->setId(paramId);
    slider->setStartPos(startPosX, startPosY);
    slider->setEndPos(endPosX, endPosY);
    slider->setRange(MinatonParams::paramMinValue(paramId), MinatonParams::paramMaxValue(paramId));
    slider->setStep(step);
    slider->setValue(MinatonParams::paramDefaultValue(paramId));
    slider->setInverted(inverted);
    slider->setCallback(this);
#endif
}

void CCetoneUI::_createSwitchButton(ScopedPointer<ImageSwitch>& switchButton, uint32_t paramId, uint absolutePosX, uint absolutePosY)
{
    switchButton = new ImageSwitch(this, fImgSwitchButton_OFF, fImgSwitchButton_ON);
    switchButton->setId(paramId);
    switchButton->setAbsolutePos(absolutePosX, absolutePosY);
    switchButton->setCallback(this);
}

void CCetoneUI::_createButton(ScopedPointer<ImageButton>& button, uint id, Image& imageNormal, Image& imagePressed, uint absolutePosX, uint absolutePosY)
{
    button = new ImageButton(this, imageNormal, imagePressed);
    button->setId(id);
    button->setAbsolutePos(absolutePosX, absolutePosY);
    button->setCallback(this);
}

const char* CCetoneUI::_wave2Str(int wave)
{
    switch (wave) {
    case WAVE_SAW:
        return "Saw";
        break;
    case WAVE_PULSE:
        return "Pulse";
        break;
    case WAVE_TRI:
        return "Tri";
        break;
    default:
        return "Unknown";
        break;
    }
}

const char* CCetoneUI::_filterType2Str(int type)
{
    switch (type) {
    case FILTER_TYPE_BIQUAD:
        return "Biquad";
        break;
    case FILTER_TYPE_MOOG:
        return "Moogle";
        break;
    default:
        return "Unknown";
        break;
    }
}

int CCetoneUI::_pf2i(float val, int max)
{
    int tmp = (int)floor(((float)(max + 1) * (float)val) + 0.5f);

    if (tmp < 0)
        tmp = 0;
    else if (tmp > max)
        tmp = max;

    return tmp;
}

float CCetoneUI::_pi2f(int val, int max)
{
    return (float)val / (float)(max + 1);
}

int CCetoneUI::_c_val2coarse(float value)
{
    return (int)(value * 100.f + 0.5f) - 50;
}

int CCetoneUI::_c_val2fine(float value)
{
    return (int)(value * 200.f + 0.5f) - 100;
}

int CCetoneUI::_c_val2pw(float value)
{
    return (int)(value * 65536.f + 0.5f);
}

int CCetoneUI::_c_val2modAmount(float value)
{
    return floorf(value * 200.f + 0.5f) - 100.f;
}

int CCetoneUI::_c_val2modMul(float value)
{
    return floorf(value * 100.f + 0.5f);
}

#ifdef ENABLE_PRESET_MENU
extern unsigned char PresetData[];

inline float c_coarse2val(int value)
{
	return (float)(value + 50) / 100.f;
}

inline float c_fine2val(int value)
{
	return (float)(value + 100) / 200.f;
}

inline float c_bool2val(bool value)
{
	return (value) ? 1.f : 0.f;
}

void CCetoneUI::_initFactoryBank()
{
    // Copy preset data to structure
    memcpy(this->fFactoryPresets, PresetData, sizeof(SynthProgramOld) * 128);
}

#define APPLY_PARAM(id, val) \
    setParameterValue(id, val); /* Apply parameter */ \
    parameterChanged(id, val);  /* Update UI display */

void CCetoneUI::_loadPreset(uint32_t presetId)
{
    DISTRHO_SAFE_ASSERT(presetId >= 0 && presetId < 128)

    const SynthProgramOld &currentPreset = fFactoryPresets[presetId];

    APPLY_PARAM(pOsc1Coarse, c_coarse2val(currentPreset.Coarse[0]));
    APPLY_PARAM(pOsc1Fine, c_fine2val(currentPreset.Fine[0]));
    APPLY_PARAM(pOsc1Wave, _pi2f(currentPreset.Wave[0], WAVE_MAX));
    APPLY_PARAM(pOsc1Morph, currentPreset.Morph[0]);
    APPLY_PARAM(pOsc1Volume, currentPreset.Volume[0]);

    APPLY_PARAM(pOsc2Fine, c_fine2val(currentPreset.Fine[1]));
    APPLY_PARAM(pOsc2Coarse, c_coarse2val(currentPreset.Coarse[1]));
    APPLY_PARAM(pOsc2Wave, _pi2f(currentPreset.Wave[1], WAVE_MAX));
    APPLY_PARAM(pOsc2Morph, currentPreset.Morph[1]);
    APPLY_PARAM(pOsc2Volume, currentPreset.Volume[1]);    

    APPLY_PARAM(pVolume, currentPreset.MainVolume > 1.0f ? 0.5f : currentPreset.MainVolume);
    APPLY_PARAM(pClipState, c_bool2val(currentPreset.ClipState));

    APPLY_PARAM(pEnv1Attack, currentPreset.Attack[0]);
    APPLY_PARAM(pEnv1Decay, currentPreset.Decay[0]);

    APPLY_PARAM(pEnv2Attack, currentPreset.Attack[1]);
    APPLY_PARAM(pEnv2Decay, currentPreset.Decay[1]);

    APPLY_PARAM(pModEnv, currentPreset.ModEnv);
    APPLY_PARAM(pModRes, currentPreset.ModRes);
    APPLY_PARAM(pModVel, currentPreset.ModVel);

    APPLY_PARAM(pCutoff, currentPreset.Cutoff);
    APPLY_PARAM(pResonance, currentPreset.Resonance);
    APPLY_PARAM(pFilterType, 0);    // Filter type is not defined in factory preset

    APPLY_PARAM(pGlideState, c_bool2val(currentPreset.GlideState));
    APPLY_PARAM(pGlideSpeed, currentPreset.GlideSpeed);

    d_stderr("Loaded preset #%d: %s", presetId, currentPreset.Name);
}

#endif
