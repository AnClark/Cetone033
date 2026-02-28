#pragma once

#include "DistrhoUI.hpp"
#include "ImageWidgets.hpp"
#include "NanoVG.hpp"

#include "Widgets/ImGui_UI.hpp"
#include "PresetManager.h"

using DGL_NAMESPACE::ImageAboutWindow;
using DGL_NAMESPACE::ImageButton;
using DGL_NAMESPACE::ImageKnob;
using DGL_NAMESPACE::ImageSlider;
using DGL_NAMESPACE::ImageSwitch;

class MinatonPresetManager;

// -----------------------------------------------------------------------

class CCetoneUI : public DISTRHO::UI,
                  public ImageButton::Callback,
                  public ImageKnob::Callback,
                  public ImageSlider::Callback,
                  public ImageSwitch::Callback,
                  public IdleCallback {
public:
    CCetoneUI();

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char *key, const char *value) override;

    // -------------------------------------------------------------------
    // Widget Callbacks

    void imageButtonClicked(ImageButton* button, int) override;
    void imageSwitchClicked(ImageSwitch* button, bool) override;
    void imageKnobDragStarted(ImageKnob* knob) override;
    void imageKnobDragFinished(ImageKnob* knob) override;
    void imageKnobValueChanged(ImageKnob* knob, float value) override;
    void imageSliderDragStarted(ImageSlider* slider) override;
    void imageSliderDragFinished(ImageSlider* slider) override;
    void imageSliderValueChanged(ImageSlider* slider, float value) override;

    void onDisplay() override;

    // -------------------------------------------------------------------
    // Other Callbacks

    void idleCallback() override;

    // -------------------------------------------------------------------
    // UI Tools (only invoked by CCetoneUI and its friend classes)

    void logAndShowMessage(const char* fmt, ...);

private:
    // -------------------------------------------------------------------
    // Label renderer

    NanoVG fNanoText;
    char fLabelBuffer[32 + 1];

    // -------------------------------------------------------------------
    // Dear ImGui Instance

    ScopedPointer<ImGuiUI> fImGuiInstance;
    friend class ImGuiUI;

    ScopedPointer<ImGuiAboutWindow> fImGuiAboutWindow;

    // -------------------------------------------------------------------
    // Image resources

    Image fImgBackground;
    Image fImgKnob;
    Image fImgSwitchButton_ON, fImgSwitchButton_OFF;
    Image fImgTransparent;

    // -------------------------------------------------------------------
    // Widgets

    ScopedPointer<ImageKnob> fKnobOsc1Coarse, fKnobOsc1Fine, fKnobOsc1Waveform, fKnobOsc1Morph, fKnobOsc1Volume;
    ScopedPointer<ImageKnob> fKnobOsc2Coarse, fKnobOsc2Fine, fKnobOsc2Waveform, fKnobOsc2Morph, fKnobOsc2Volume;

    ScopedPointer<ImageKnob> fKnobVolume;

    ScopedPointer<ImageKnob> fKnobGlideSpeed;

    ScopedPointer<ImageKnob> fAmpAttack, fAmpDecay;
    ScopedPointer<ImageKnob> fModAttack, fModDecay;

    ScopedPointer<ImageKnob> fLfoSpeed, fLfoWaveform, fLfoPulseWidth;

    ScopedPointer<ImageKnob> fModEnvelope, fModVelocity, fModResDecay;

    ScopedPointer<ImageKnob> fFilterCutoff, fFilterResonance, fFilterType;

    ScopedPointer<ImageSwitch> fBtnClipState, fBtnGlideState;

    // -------------------------------------------------------------------
    // Buttons

    ScopedPointer<ImageButton> fBtnAbout;

    ScopedPointer<ImageButton> fBtnOsc1Waveform, fBtnOsc2Waveform;
    ScopedPointer<ImageButton> fBtnFilterType;

    // -------------------------------------------------------------------
    // Special parameters' storage (not controlled by knobs and switches)

#ifdef ENABLE_POLYPHONY
    uint32_t fMaxPolyphony;
#endif

    // -------------------------------------------------------------------
    // Preset Manager Instance

    // TODO: Save preset name in plugin state for preset switching and recall.
    

    ScopedPointer<CPresetManager> fPresetManager;
    friend class CPresetManager;
    String fCurrentPresetName;
    String fCurrentPresetBank;  // Bank name: FACTORY_BANK_NAME, DEFAULT_USER_BANK_NAME, BANK_NAME_FOR_SINGLE_IMPORTED_PRESET or any imported bank name
    bool fPresetIsModified; // TODO: Append asterisk to preset name when current program is modified but not saved, like "Init Patch*".
                            // This should be stored in plugin state as well, to avoid losing this info when reopening UI.

    // -------------------------------------------------------------------
    // Helpers

    void _createKnob(ScopedPointer<ImageKnob>& knob, uint32_t paramId, uint absolutePosX, uint absolutePosY, float defaultValue, uint rotationAngle = 275);
    void _createSlider(ScopedPointer<ImageSlider>& slider, uint32_t paramId, uint startPosX, uint startPosY, uint endPosX, uint endPosY, float step, bool inverted = false);
    void _createSwitchButton(ScopedPointer<ImageSwitch>& switchButton, uint32_t paramId, uint absolutePosX, uint absolutePosY);
    void _createButton(ScopedPointer<ImageButton>& button, uint id, Image& imageNormal, Image& imagePressed, uint absolutePosX, uint absolutePosY);
    void _createHiddenButton(ScopedPointer<ImageButton>& button, uint id, Size<uint> size, Point<int> absolutePos);

    void _requestMessageBox(std::string message);

    const char* _wave2Str(int wave);
    const char* _filterType2Str(int type);  // Equals CCetoneLpFilter::Name()

    int _pf2i(float val, int max);
    float _pi2f(int val, int max);
    int _c_val2coarse(float value);
    int _c_val2fine(float value);
    int _c_val2pw(float value);
    int _c_val2modAmount(float value);
    int _c_val2modMul(float value);

    void _updateState(const char* newPresetName, const char* newBankName, bool isModified);
    void _updateState(bool isModified);
    void _triggerDummyParameterChange();

    // Returns true when fCurrentPresetName exists within fCurrentPresetBank on disk.
    // Used by stateChanged() to detect and correct stale state pushed back by host undo.
    bool fPresetNameStateChecked, fBankNameStateChecked;    // Mark whether we've received preset name and bank name at least once from host,
                                                            // so we know when to start validating the state.
    String fPendingPresetName, fPendingBankName;    // Temporarily store the preset name and bank name received from host for validation,
                                                    // before copying them to fCurrentPresetName and fCurrentPresetBank.
    bool _validatePresetAndBankState(const String& presetName, const String& bankName); // Check if the given preset name and bank name are valid
                                                                                        // (exist on disk). Returns true if valid, false if not.

    // Resets UI preset metadata to the factory default state.  Called when
    // stateChanged() detects that the host pushed back a bank or preset that no
    // longer exists on disk (e.g. after the user deleted it this session).
    // Does NOT modify parameter values – those are correctly restored by the host.
    void _fallbackToDefaultStateOfPreset();
    void _fallbackToDefaultStateOfBank();

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCetoneUI)
};

// -----------------------------------------------------------------------

// --------------------------------
// Button IDs

constexpr uint BTN_PANIC = d_cconst('p', 'n', 'i', 'c');
constexpr uint BTN_ABOUT = d_cconst('a', 'b', 't', '.');

// -----------------------------------------------------------------------
