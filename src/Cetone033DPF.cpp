#include "Cetone033.h"

void CCetone033::initParameter(uint32_t index, Parameter& parameter)
{
    parameter.hints |= kParameterIsAutomatable;

    switch (index) {
    case pOsc1Volume:
    case pOsc2Volume:
    case pVolume:
        // For volume parameters, param values are factors for calculating amplifier parameters.
        // They are not limited to range of 0.0 ~ 1.0, but up to 2.0 to allow boost,
        // which is the original plugin's behavior (some factory patches do).
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.5f;    // Allow 50% boost
        parameter.ranges.def = getParameter(index);
        break;

    default:
        // For other parameters, fallback to classic VST 2.4 param range (0.0 ~ 1.0),
        // to fit with Cetone's own param handlers.
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = getParameter(index);
    }

    // Must set parameter.symbol, this is the unique ID of each parameter.
    // If not set, you can neither save presets nor reset to factory default, in VST3 and CLAP!
    char buff[256];
    getParameterName(index, buff);
    parameter.symbol = String(buff).replace(' ', '_').replace('.', '_');
    parameter.name = String(buff);

    switch (index) {
    case pClipState:
    case pGlideState:
        parameter.hints |= kParameterIsBoolean;
        break;
    }
}

float CCetone033::getParameterValue(uint32_t index) const
{
    return this->getParameter(index);
}

void CCetone033::setParameterValue(uint32_t index, float value)
{
    this->setParameter(index, value);
}

void CCetone033::activate()
{
    this->resume();
}

void CCetone033::run(const float** inputs, float** outputs, uint32_t frames, const DISTRHO::MidiEvent* midiEvents, uint32_t midiEventCount)
{
    this->processEvents(midiEvents, midiEventCount);
    this->processReplacing((float**)inputs, outputs, frames);
}

void CCetone033::sampleRateChanged(double newSampleRate)
{
    this->setSampleRate(newSampleRate);
}

void CCetone033::bufferSizeChanged(int newBufferSize)
{
    this->setBlockSize(newBufferSize);
}
