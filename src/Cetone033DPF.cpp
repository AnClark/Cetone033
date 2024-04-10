#include "Cetone033.h"

void CCetone033::initParameter(uint32_t index, Parameter& parameter)
{
    parameter.hints |= kParameterIsAutomatable;

    // Fallback to classic VST 2.4 param range (0.0 ~ 1.0), to fit with Cetone's own param handlers.
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.5f;

    char buff[256];
    getParameterName(index, buff);
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
