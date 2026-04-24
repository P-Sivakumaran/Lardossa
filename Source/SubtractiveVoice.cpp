#include "SubtractiveVoice.h"
#include "Saturators.h"
#include <cmath>
#include <cstdint>

namespace
{
inline float polyBLEP(float phase, float dt)
{
    if (phase < dt)
    {
        const auto t = phase / dt;
        return t + t - t * t - 1.0f;
    }
    if (phase > 1.0f - dt)
    {
        const auto t = (phase - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}
} // namespace

void SubtractiveVoice::startNote(int midiNoteNumber, float velocity, bool accentStep, bool legatoNoRetrigger, bool slideStep)
{
    targetFreq = (float) juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    level = juce::jlimit(0.0f, 1.0f, velocity);
    noteVelocity = level;
    accentActive = accentStep;
    noteActive = true;
    glideActive = true;

    if (slideStep)
    {
        const float effMs = juce::jmax(lastGlideMs, 110.0f);
        glideCoef = std::exp(-1.0f / (effMs * 0.001f * sr));
    }

    if (!legatoNoRetrigger)
    {
        if (currentFreq <= 0.01f)
            currentFreq = targetFreq;
        triState = 0.0f;
        ampEnvelope.noteOn();
        filterEnvelope.noteOn();
    }
}

void SubtractiveVoice::stopNote()
{
    ampEnvelope.noteOff();
    filterEnvelope.noteOff();
}

void SubtractiveVoice::setSampleRate(double newSampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec{newSampleRate, (uint32_t) maxBlockSize, 1};
    filter.prepare(spec);
    filter.reset();
    ampEnvelope.setSampleRate(newSampleRate);
    filterEnvelope.setSampleRate(newSampleRate);
    sr = (float) newSampleRate;
    tempBuffer.setSize(1, maxBlockSize, false, false, false);
}

void SubtractiveVoice::updateParams(int oscShape,
                                    float subLevel_,
                                    float pulseWidth_,
                                    float osc2Detune_,
                                    float osc2Mix_,
                                    float noiseLevel_,
                                    float filterDrive_,
                                    float glideMs_,
                                    float cutoffHz,
                                    float resonance,
                                    float filterEnvAmount,
                                    float filterDecayMs,
                                    float ampAttackMs,
                                    float ampDecayMs,
                                    float ampSustain,
                                    float ampReleaseMs)
{
    waveShape = oscShape;
    subLevel = subLevel_;
    pulseWidth = juce::jlimit(0.05f, 0.95f, pulseWidth_);
    osc2Detune = osc2Detune_;
    osc2Mix = juce::jlimit(0.0f, 1.0f, osc2Mix_);
    noiseLevel = noiseLevel_;
    filterDrive = filterDrive_;

    osc2FreqRatio = std::pow(2.0f, osc2Detune_ / 1200.0f);
    lastGlideMs = glideMs_;
    glideCoef = glideMs_ > 0.1f ? std::exp(-1.0f / (glideMs_ * 0.001f * sr)) : 0.0f;
    driveGain = 1.0f + filterDrive * 4.0f;
    invTanhDriveGain = filterDrive > 0.01f ? 1.0f / std::tanh(driveGain) : 1.0f;

    baseCutoff = cutoffHz;
    envAmount = filterEnvAmount;

    voiceResonance = juce::jlimit(0.05f, 0.99f, resonance);
    filter.setMode(juce::dsp::LadderFilterMode::LPF24);
    filter.setDrive(1.0f + filterDrive * 3.0f);

    juce::ADSR::Parameters amp;
    amp.attack = ampAttackMs / 1000.0f;
    amp.decay = ampDecayMs / 1000.0f;
    amp.sustain = ampSustain;
    amp.release = ampReleaseMs / 1000.0f;
    ampEnvelope.setParameters(amp);

    juce::ADSR::Parameters fenv;
    fenv.attack = 0.0005f;
    fenv.decay = juce::jmax(0.005f, filterDecayMs / 1000.0f);
    fenv.sustain = 0.0f;
    fenv.release = 0.015f;
    filterEnvelope.setParameters(fenv);
}

void SubtractiveVoice::renderAddingTo(juce::AudioBuffer<float>& outputBuffer, int destStartSample, int numSamples)
{
    if (sr <= 0.0f || numSamples <= 0 || !noteActive)
        return;

    if (tempBuffer.getNumSamples() < numSamples)
        return;

    const auto velocityBoost = accentActive ? (0.62f + (0.95f * noteVelocity)) : (0.55f + (0.85f * noteVelocity));
    const auto accentFilterMul = accentActive ? 1.72f : 1.0f;
    filter.setResonance(juce::jlimit(0.05f, 0.99f,
                                     voiceResonance * (accentActive ? 1.28f : 1.0f)));

    float* buf = tempBuffer.getWritePointer(0);
    juce::dsp::AudioBlock<float> fullBlock(tempBuffer);

    for (int i = 0; i < numSamples; ++i)
    {
        if (glideCoef > 0.0f)
        {
            currentFreq += (1.0f - glideCoef) * (targetFreq - currentFreq);
            if (std::abs(currentFreq - targetFreq) < 0.1f)
                currentFreq = targetFreq;
        }
        else
            currentFreq = targetFreq;

        const float dt1 = currentFreq / sr;
        const float dt2 = currentFreq * osc2FreqRatio / sr;
        const float dtSub = currentFreq * 0.5f / sr;

        phase1 += dt1;
        if (phase1 >= 1.0f)
            phase1 -= 1.0f;
        const float osc1 = renderOsc(phase1, dt1, waveShape);

        phase2 += dt2;
        if (phase2 >= 1.0f)
            phase2 -= 1.0f;
        const float osc2 = renderOsc(phase2, dt2, waveShape);

        subPhase += dtSub;
        if (subPhase >= 1.0f)
            subPhase -= 1.0f;
        float sub = (subPhase < 0.5f) ? 1.0f : -1.0f;
        sub += polyBLEP(subPhase, dtSub);
        sub -= polyBLEP(std::fmod(subPhase + 0.5f, 1.0f), dtSub);

        noiseState ^= noiseState << 13;
        noiseState ^= noiseState >> 17;
        noiseState ^= noiseState << 5;
        const float noise = ((float) (int32_t) noiseState) / (float) INT32_MAX;

        const float oscBlend = juce::jmap(osc2Mix, osc1, osc2);
        float sample = (oscBlend + sub * subLevel + noise * noiseLevel) * level;

        if (filterDrive > 0.01f)
            sample = lardossa::dsp::fastTanh(sample * driveGain) * invTanhDriveGain;

        buf[i] = sample;
    }

    constexpr int kFilterPeriod = 2;
    for (int i = 0; i < numSamples; i += kFilterPeriod)
    {
        const int chunk = juce::jmin(kFilterPeriod, numSamples - i);
        float fenv = 0.0f;
        for (int j = 0; j < chunk; ++j)
            fenv = filterEnvelope.getNextSample();

        const float dynCutoff = juce::jlimit(30.0f, 18000.0f,
                                             baseCutoff + envAmount * velocityBoost * accentFilterMul * 16000.0f * fenv);
        filter.setCutoffFrequencyHz(dynCutoff);

        auto block = fullBlock.getSubBlock((size_t) i, (size_t) chunk);
        juce::dsp::ProcessContextReplacing<float> context(block);
        filter.process(context);
    }

    for (int i = 0; i < numSamples; ++i)
        buf[i] *= ampEnvelope.getNextSample();

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        outputBuffer.addFrom(channel, destStartSample, tempBuffer, 0, 0, numSamples);

    if (!ampEnvelope.isActive())
    {
        noteActive = false;
        glideActive = false;
        accentActive = false;
    }
}

void SubtractiveVoice::render(juce::AudioBuffer<float>& outputBuffer, int numSamples)
{
    renderAddingTo(outputBuffer, 0, numSamples);
}

float SubtractiveVoice::renderOsc(float phase, float dt, int shape)
{
    switch (shape)
    {
        case 0:
        {
            float saw = 2.0f * phase - 1.0f;
            saw -= polyBLEP(phase, dt);
            return saw;
        }
        case 1:
        {
            float sq = (phase < 0.5f) ? 1.0f : -1.0f;
            sq += polyBLEP(phase, dt);
            sq -= polyBLEP(std::fmod(phase + 0.5f, 1.0f), dt);
            return sq;
        }
        case 2:
        {
            float pw = (phase < pulseWidth) ? 1.0f : -1.0f;
            pw += polyBLEP(phase, dt);
            pw -= polyBLEP(std::fmod(phase + (1.0f - pulseWidth), 1.0f), dt);
            return pw;
        }
        case 3:
        {
            float sq = (phase < 0.5f) ? 1.0f : -1.0f;
            sq += polyBLEP(phase, dt);
            sq -= polyBLEP(std::fmod(phase + 0.5f, 1.0f), dt);
            triState += 4.0f * dt * sq;
            triState = juce::jlimit(-1.0f, 1.0f, triState);
            return triState;
        }
        default:
            return 2.0f * phase - 1.0f;
    }
}
