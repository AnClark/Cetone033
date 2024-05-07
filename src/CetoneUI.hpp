#pragma once

#include "DistrhoUI.hpp"
#include "ImageWidgets.hpp"
#include "NanoVG.hpp"

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
    void programLoaded(uint32_t index) override;

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

private:
    // -------------------------------------------------------------------
    // Label renderer

    NanoVG fNanoText;
    char fLabelBuffer[32 + 1];

    // -------------------------------------------------------------------
    // Image resources

    Image fImgBackground;
    Image fImgKnob;
    Image fImgSwitchButton_ON, fImgSwitchButton_OFF;

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
    // Helpers

    void _createKnob(ScopedPointer<ImageKnob>& knob, uint32_t paramId, uint absolutePosX, uint absolutePosY, float defaultValue, uint rotationAngle = 275);
    void _createSlider(ScopedPointer<ImageSlider>& slider, uint32_t paramId, uint startPosX, uint startPosY, uint endPosX, uint endPosY, float step, bool inverted = false);
    void _createSwitchButton(ScopedPointer<ImageSwitch>& switchButton, uint32_t paramId, uint absolutePosX, uint absolutePosY);
    void _createButton(ScopedPointer<ImageButton>& button, uint id, Image& imageNormal, Image& imagePressed, uint absolutePosX, uint absolutePosY);

    const char* _wave2Str(int wave);
    const char* _filterType2Str(int type);  // Equals CCetoneLpFilter::Name()

    int _pf2i(float val, int max);
    float _pi2f(int val, int max);
    int _c_val2coarse(float value);
    int _c_val2fine(float value);
    int _c_val2pw(float value);
    int _c_val2modAmount(float value);
    int _c_val2modMul(float value);

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCetoneUI)
};

// -----------------------------------------------------------------------

// --------------------------------
// Button IDs

constexpr uint BTN_PANIC = d_cconst('p', 'n', 'i', 'c');

// -----------------------------------------------------------------------
