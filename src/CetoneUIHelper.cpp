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
