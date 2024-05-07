#include "CetoneUI.hpp"
#include "defines.h"
#include "Fonts/CetoneFonts.hpp"
#include "images/CetoneArtwork.hpp"
#include "structures.h"

namespace Art   = CetoneArtwork;
namespace Fonts = CetoneFonts;

CCetoneUI::CCetoneUI()
    : DISTRHO::UI(Art::guiWidth, Art::guiHeight)
    , fImgBackground(Art::guiData, Art::guiWidth, Art::guiHeight, kImageFormatBGR)
    , fImgKnob(Art::knobData, Art::knobWidth, Art::knobHeight, kImageFormatBGRA)
    , fImgSwitchButton_ON(Art::buttons_onData, Art::buttons_onWidth, Art::buttons_onHeight, kImageFormatBGR)
    , fImgSwitchButton_OFF(Art::buttons_offData, Art::buttons_offWidth, Art::buttons_offHeight, kImageFormatBGR)
{
    /* Initialize NanoVG font and text buffer */
    NanoVG::FontId font = fNanoText.createFontFromMemory("Source Sans Regular", Fonts::SourceSans3_RegularData, Fonts::SourceSans3_RegularDataSize, false);
    fNanoText.fontFaceId(font);
    memset(fLabelBuffer, '\0', sizeof(char) * (32 + 1));

    /* Initialize knobs */
    // NOTICE: Default values comes from CCetoneSynth::InitSynthParameters().
    //         To fit with DSP side on actual run, some values here are different from InitSynthParameters().

    // NOTICE: Volume algorithm here is different from CetoneSynthLight.
    constexpr float DEFAULT_VOLUME_PARAM_VALUE = 0.5f;

    _createKnob(fKnobVolume, pVolume, 584, 68, DEFAULT_VOLUME_PARAM_VALUE);

    _createKnob(fKnobOsc1Coarse, pOsc1Coarse, 8, 68, 0.5f);    // 0
    _createKnob(fKnobOsc1Fine, pOsc1Fine, 8 + 48, 68, 0.5f);
    _createKnob(fKnobOsc1Waveform, pOsc1Wave, 8 + 48 * 2, 68, _pi2f(WAVE_SAW, WAVE_MAX));
    _createKnob(fKnobOsc1Morph, pOsc1Morph, 8 + 48 * 3, 68, 0.5f);
    _createKnob(fKnobOsc1Volume, pOsc1Volume, 8 + 48 * 4, 68, DEFAULT_VOLUME_PARAM_VALUE);

    _createKnob(fKnobOsc2Coarse, pOsc2Coarse, 272, 68, 0.38f);
    _createKnob(fKnobOsc2Fine, pOsc2Fine, 272 + 48, 68, 0.5f);
    _createKnob(fKnobOsc2Waveform, pOsc2Wave, 272 + 48 * 2, 68, _pi2f(WAVE_PULSE, WAVE_MAX));
    _createKnob(fKnobOsc2Morph, pOsc2Morph, 272 + 48 * 3, 68, 0.5f);
    _createKnob(fKnobOsc2Volume, pOsc2Volume, 272 + 48 * 4, 68, DEFAULT_VOLUME_PARAM_VALUE);

    _createKnob(fKnobGlideSpeed, pGlideSpeed, 584, 178, 0.0f);

    _createKnob(fAmpAttack, pEnv1Attack, 8, 178, 0.02f);
    _createKnob(fAmpDecay, pEnv1Decay, 8 + 48, 178, 0.40f);

    _createKnob(fModAttack, pEnv2Attack, 116, 178, 0.02f);
    _createKnob(fModDecay, pEnv2Decay, 116 + 48, 178, 0.15f);

    _createKnob(fModEnvelope, pModEnv, 224, 178, 0.7f);
    _createKnob(fModVelocity, pModVel, 224 + 48, 178, 0.0f);
    _createKnob(fModResDecay, pModRes, 224 + 48 * 2, 178, 0.0f);

    _createKnob(fFilterCutoff, pCutoff, 380, 178, 1.0f);
    _createKnob(fFilterResonance, pResonance, 380 + 48, 178, 0.0f);
    _createKnob(fFilterType, pFilterType, 380 + 48 * 2, 178, 0.0f);

    /* Initialize switches */
    _createSwitchButton(fBtnClipState, pClipState, 536, 68);
    _createSwitchButton(fBtnGlideState, pGlideState, 536, 178);

#ifdef ENABLE_PRESET_MENU
    _initFactoryBank();
    _initPresetMenu();

    //for (auto i = 0; i < 10; i++)
    //    d_stderr("Preset #%d: %s", i, fFactoryPresets[i].Name);
#endif
}

void CCetoneUI::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case pOsc1Coarse:
        fKnobOsc1Coarse->setValue(value);
        break;
    case pOsc1Fine:
        fKnobOsc1Fine->setValue(value);
        break;
    case pOsc1Wave:
        fKnobOsc1Waveform->setValue(value);
        break;
    case pOsc1Morph:
        fKnobOsc1Morph->setValue(value);
        break;
    case pOsc1Volume:
        fKnobOsc1Volume->setValue(value);
        break;

    case pOsc2Coarse:
        fKnobOsc2Coarse->setValue(value);
        break;
    case pOsc2Fine:
        fKnobOsc2Fine->setValue(value);
        break;
    case pOsc2Wave:
        fKnobOsc2Waveform->setValue(value);
        break;
    case pOsc2Morph:
        fKnobOsc2Morph->setValue(value);
        break;
    case pOsc2Volume:
        fKnobOsc2Volume->setValue(value);
        break;

    case pEnv1Attack:
        fAmpAttack->setValue(value);
        break;
    case pEnv1Decay:
        fAmpDecay->setValue(value);
        break;

    case pEnv2Attack:
        fModAttack->setValue(value);
        break;
    case pEnv2Decay:
        fModDecay->setValue(value);
        break;

    case pModEnv:
        fModEnvelope->setValue(value);
        break;
    case pModVel:
        fModVelocity->setValue(value);
        break;
    case pModRes:
        fModResDecay->setValue(value);
        break;

    case pCutoff:
        fFilterCutoff->setValue(value);
        break;
    case pResonance:
        fFilterResonance->setValue(value);
        break;

    case pGlideState:
        fBtnGlideState->setDown(value == 1.0f ? true : false);
        break;
    case pGlideSpeed:
        fKnobGlideSpeed->setValue(value);
        break;

    case pClipState:
        fBtnClipState->setDown(value == 1.0f ? true : false);
        break;
    case pVolume:
        fKnobVolume->setValue(value);
        break;

    case pFilterType:
        fFilterType->setValue(value);
        break;

    default:
        d_stderr2("WARNING: unrecognized parameter %d", index);
        break;
    }

    // Explicitly ask DPF to redraw UI (for updating labels)
    repaint();
}

// -------------------------------------------------------------------
// Widget Callbacks

void CCetoneUI::imageButtonClicked(ImageButton* button, int)
{
#if 0
    switch (button->getId()) {
    case BTN_PANIC: {
        panic();
        break;
    }
    }
#endif
}

void CCetoneUI::imageSwitchClicked(ImageSwitch* button, bool down)
{
    setParameterValue(button->getId(), down);
}

void CCetoneUI::imageKnobDragStarted(ImageKnob* knob)
{
    editParameter(knob->getId(), true);
}

void CCetoneUI::imageKnobDragFinished(ImageKnob* knob)
{
    editParameter(knob->getId(), false);
}

void CCetoneUI::imageKnobValueChanged(ImageKnob* knob, float value)
{
    setParameterValue(knob->getId(), value);

    // Explicitly ask DPF to redraw UI (for updating labels)
    repaint();
}

void CCetoneUI::imageSliderDragStarted(ImageSlider* slider)
{
    editParameter(slider->getId(), true);
}

void CCetoneUI::imageSliderDragFinished(ImageSlider* slider)
{
    editParameter(slider->getId(), false);
}

void CCetoneUI::imageSliderValueChanged(ImageSlider* slider, float value)
{
    setParameterValue(slider->getId(), value);
}

void CCetoneUI::onDisplay()
{
    /**
     * Draw background
     */
    const GraphicsContext& context(getGraphicsContext());

    fImgBackground.draw(context);

    /**
     * Draw labels
     * NOTICE: Must invoke `repaint()` when tuning widgets, otherwise UI won't update.
     */
    constexpr float r = 255.0f;
    constexpr float g = 255.0f;
    constexpr float b = 255.0f;

    fNanoText.beginFrame(this);
    fNanoText.fontSize(18);
    fNanoText.textAlign(NanoVG::ALIGN_CENTER | NanoVG::ALIGN_MIDDLE);

    fNanoText.fillColor(Color(r, g, b));

    /* Oscillator 1 */
    // Osc1 Coarse
    std::snprintf(fLabelBuffer, 32, "%d", _c_val2coarse(fKnobOsc1Coarse->getValue()));
    fNanoText.textBox(10, 124, 45.0f, fLabelBuffer);

    // Osc1 Fine
    std::snprintf(fLabelBuffer, 32, "%d", _c_val2fine(fKnobOsc1Fine->getValue()));
    fNanoText.textBox(10 + 48, 124, 45.0f, fLabelBuffer); 

    // Osc1 Waveform
    std::snprintf(fLabelBuffer, 32, "%s", _wave2Str(_pf2i(fKnobOsc1Waveform->getValue(), WAVE_MAX)));
    fNanoText.textBox(10 + 48 * 2, 124, 45.0f, fLabelBuffer);

    // Osc1 Morph
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobOsc1Morph->getValue() * 100.0f);
    fNanoText.textBox(10 + 48 * 3, 124, 45.0f, fLabelBuffer);

    // Osc1 Volume
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobOsc1Volume->getValue() * 100.0f);
    fNanoText.textBox(10 + 48 * 4, 124, 45.0f, fLabelBuffer);

    /* Oscillator 2 */
    // Osc2 Coarse
    std::snprintf(fLabelBuffer, 32, "%d", _c_val2coarse(fKnobOsc2Coarse->getValue()));
    fNanoText.textBox(274, 124, 45.0f, fLabelBuffer);

    // Osc2 Fine
    std::snprintf(fLabelBuffer, 32, "%d", _c_val2fine(fKnobOsc2Fine->getValue()));
    fNanoText.textBox(274 + 48, 124, 45.0f, fLabelBuffer); 

    // Osc2 Waveform
    std::snprintf(fLabelBuffer, 32, "%s", _wave2Str(_pf2i(fKnobOsc2Waveform->getValue(), WAVE_MAX)));
    fNanoText.textBox(274 + 48 * 2, 124, 45.0f, fLabelBuffer);

    // Osc2 Morph
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobOsc2Morph->getValue() * 100.0f);
    fNanoText.textBox(274 + 48 * 3, 124, 45.0f, fLabelBuffer);

    // Osc2 Volume
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobOsc2Volume->getValue() * 100.0f);
    fNanoText.textBox(274 + 48 * 4, 124, 45.0f, fLabelBuffer);

    /* Mixer */
    // Main volume
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobVolume->getValue() * 100.0f);
    fNanoText.textBox(586, 124, 45.0f, fLabelBuffer);

    /* Amplifier Envelope */
    // Attack
    std::snprintf(fLabelBuffer, 32, "%.2f", fAmpAttack->getValue() * 100.0f);
    fNanoText.textBox(10, 234, 45.0f, fLabelBuffer);

    // Decay
    std::snprintf(fLabelBuffer, 32, "%.2f", fAmpDecay->getValue() * 100.0f);
    fNanoText.textBox(10 + 48, 234, 45.0f, fLabelBuffer);

    /* Modulation Envelope */
    // Attack
    std::snprintf(fLabelBuffer, 32, "%.2f", fModAttack->getValue() * 100.0f);
    fNanoText.textBox(118, 234, 45.0f, fLabelBuffer);

    // Decay
    std::snprintf(fLabelBuffer, 32, "%.2f", fModDecay->getValue() * 100.0f);
    fNanoText.textBox(118 + 48, 234, 45.0f, fLabelBuffer);

    /* Filter Modulation */
    // Mod Envelope
    std::snprintf(fLabelBuffer, 32, "%.2f", fModEnvelope->getValue() * 100.0f);
    fNanoText.textBox(226, 234, 45.0f, fLabelBuffer);

    // Velocity
    std::snprintf(fLabelBuffer, 32, "%.2f", fModVelocity->getValue() * 100.0f);
    fNanoText.textBox(226 + 48, 234, 45.0f, fLabelBuffer);

    // Res Decay
    std::snprintf(fLabelBuffer, 32, "%.2f", fModResDecay->getValue() * 100.0f);
    fNanoText.textBox(226 + 48 * 2, 234, 45.0f, fLabelBuffer);

    /* Filter */
    // Cutoff
    std::snprintf(fLabelBuffer, 32, "%.2f", fFilterCutoff->getValue() * 100.0f);
    fNanoText.textBox(382, 234, 45.0f, fLabelBuffer);

    // Resonance
    std::snprintf(fLabelBuffer, 32, "%.2f", fFilterResonance->getValue() * 100.0f);
    fNanoText.textBox(382 + 48, 234, 45.0f, fLabelBuffer);

    // Filter type
    std::snprintf(fLabelBuffer, 32, "%s", _filterType2Str(_pf2i(fFilterType->getValue(), FILTER_TYPE_MAX)));
    fNanoText.textBox(382 + 48 * 2, 234, 45.0f, fLabelBuffer);

    /* Glide (portamento) */
    // Speed
    std::snprintf(fLabelBuffer, 32, "%.1f", fKnobGlideSpeed->getValue() * 100.0f);
    fNanoText.textBox(586, 234, 45.0f, fLabelBuffer); 

    fNanoText.endFrame();
}

void CCetoneUI::idleCallback() { }

// -----------------------------------------------------------------------

START_NAMESPACE_DISTRHO

UI* createUI()
{
    return new CCetoneUI();
}

END_NAMESPACE_DISTRHO
