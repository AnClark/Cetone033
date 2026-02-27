#include "CetoneUI.hpp"
#include "defines.h"

constexpr float PARAM_MIN_VALUE = 0.0f;
constexpr float PARAM_MAX_VALUE = 1.0f;
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
constexpr float PARAM_MAX_VALUE_VOLUME = 5.0f;
#endif
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

#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    switch (paramId) {
    case pOsc1Volume:
    case pOsc2Volume:
    case pVolume:
        knob->setRange(PARAM_MIN_VALUE, PARAM_MAX_VALUE_VOLUME);
        break;
    }
#endif
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

void CCetoneUI::_createHiddenButton(ScopedPointer<ImageButton>& button, uint id, Size<uint> size, Point<int> absolutePos)
{
    button = new ImageButton(this, fImgTransparent, fImgTransparent);
    button->setId(id);
    button->setAbsolutePos(absolutePos);
    button->setSize(size);
    button->setCallback(this);
}

void CCetoneUI::_requestMessageBox(std::string message)
{
    DISTRHO_SAFE_ASSERT_RETURN(fImGuiInstance.get(), )

    // Append new message to message box queue.
    // UI polls message queue on every OnImGuiDisplay() call, then show message box on demand.
    fImGuiInstance->messageBoxQueue.push(std::string(message));
}

void CCetoneUI::logAndShowMessage(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    constexpr uint16_t MAX_MESSAGE_LENGTH = 512;
    char buffer[MAX_MESSAGE_LENGTH] = {'\0'};
    vsnprintf(buffer, MAX_MESSAGE_LENGTH, fmt, args);

    va_end(args);

    // Print log to console
    d_stderr("%s", buffer);

    // Show message box on UI side
    // NOTE: Use std::string because it supports deep copy when passing params to another function.
    //       If not using std::string (e.g. passing char[] or DISTRHO::String), object may be destroyed too early,
    //       messing up the message text in _requestMessageBox().
    _requestMessageBox(std::string(buffer));
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

void CCetoneUI::_updateState(const char* newPresetName, const char* newBankName, bool isModified)
{
    // Update local storage
    this->fCurrentPresetName = newPresetName;
    this->fCurrentPresetBank = newBankName;
    this->fPresetIsModified = isModified;

    // Send state to DSP side
    this->setState(STATE_PRESET_NAME, newPresetName);
    this->setState(STATE_PRESET_BANK, newBankName);
    this->setState(STATE_PRESET_MODIFIED, isModified ? "true" : "false");
}

void CCetoneUI::_updateState(bool isModified)
{
    this->fPresetIsModified = isModified;
    this->setState(STATE_PRESET_MODIFIED, isModified ? "true" : "false");
}

bool CCetoneUI::_validatePresetAndBankState(const String& presetName, const String& bankName)
{
    if (presetName == DEFAULT_PRESET_NAME && bankName == FACTORY_BANK_NAME) {
        // Default preset always exists in factory bank
        return true;
    }

    std::vector<String> defaultBankPresets;
    std::vector<String> importedBanks;
    std::map<std::string, std::vector<String>> importedBankPresets;

    // Fetch the newest list of presets (default bank)
    for (size_t i = 0; i < fPresetManager->getDefaultBankPresetCount(); i++)
        defaultBankPresets.push_back(fPresetManager->getDefaultBankPresetName(i));

    // Fetch the newest list of banks and presets (imported banks)
    importedBanks = fPresetManager->getImportedBankNames();
    importedBankPresets.clear();
    for (const auto& bank : importedBanks)
        importedBankPresets[bank.buffer()] = fPresetManager->getPresetsInBank(bank.buffer());

    if (bankName == FACTORY_BANK_NAME) {
        // Factory preset: check if it still exists in factory bank
        for (uint32_t i = 0; i < 128; i++) {
            if (presetName == String(fPresetManager->getFactoryProgramName(i))) {
                return true;
            }
        }
        return false;
    } else if ((bankName == BANK_NAME_FOR_SINGLE_IMPORTED_PRESET)) {
        // Single imported preset: lives in memory only (not backed by any bank file).
        // We cannot verify it on disk, so trust whatever the host says.
        return true;
    } else if (bankName == DEFAULT_USER_BANK_NAME) {
        // Default user preset: check if it still exists in default bank
        for (const auto& preset : defaultBankPresets) {
            if (presetName == preset) {
                return true;
            }
        }
        return false;
    } else {
        // Imported bank preset: check if bank and preset still exist
        auto it = importedBankPresets.find(bankName.buffer());
        if (it != importedBankPresets.end()) {
            const std::vector<String>& presets = it->second;
            for (const auto& preset : presets) {
                if (presetName == preset) {
                    return true;
                }
            }
        }
        return false;
    }
}

void CCetoneUI::_fallbackToDefaultStateOfPreset()
{
    // Correct UI metadata to default state when the host reverts to a snapshot
    // that references a bank or preset that no longer exists on disk.
    // Does NOT touch parameter values; those were already restored correctly
    // by the host's undo mechanism.
    fCurrentPresetName = DEFAULT_PRESET_NAME;
    fPresetIsModified  = true; // Parameters no longer match any saved preset
    setState(STATE_PRESET_NAME,     DEFAULT_PRESET_NAME);
    setState(STATE_PRESET_MODIFIED, "true");
}

void CCetoneUI::_fallbackToDefaultStateOfBank()
{
    // Correct UI metadata to default state when the host reverts to a snapshot
    // that references a bank or preset that no longer exists on disk.
    // Does NOT touch parameter values; those were already restored correctly
    // by the host's undo mechanism.
    fCurrentPresetBank = FACTORY_BANK_NAME;
    fPresetIsModified  = true; // Parameters no longer match any saved preset
    setState(STATE_PRESET_BANK,     FACTORY_BANK_NAME);
    setState(STATE_PRESET_MODIFIED, "true");
}
