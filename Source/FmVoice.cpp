#include "FmVoice.h"
#include <juce_core/juce_core.h>
#include <cmath>

void FmVoice::setSampleRate(float sr)
{
    sampleRate = std::max(100.f, sr);
    for (auto& o : ops)
        o.setSampleRate(sampleRate);
}

void FmVoice::setAlgorithm(int algoIndex)
{
    algorithm = juce::jlimit(0, 4, algoIndex);
}

void FmVoice::setOperatorParams(int opIdx, float ratio, float detune, float level, float feedback)
{
    if (!juce::isPositiveAndBelow(opIdx, kNumOps))
        return;
    ops[(size_t) opIdx].ratio = ratio;
    ops[(size_t) opIdx].detuneSemis = detune;
    ops[(size_t) opIdx].outputLevel = juce::jlimit(0.f, 1.f, level);
    if (opIdx == 0)
        ops[0].feedback = juce::jlimit(0.f, 1.f, feedback);
}

void FmVoice::setOperatorEnv(int opIdx, float attackMs, float decayMs, float sustain, float releaseMs)
{
    if (!juce::isPositiveAndBelow(opIdx, kNumOps))
        return;
    auto& o = ops[(size_t) opIdx];
    o.attackMs = juce::jlimit(0.5f, 4000.f, attackMs);
    o.decayMs = juce::jlimit(1.f, 4000.f, decayMs);
    o.sustainLevel = juce::jlimit(0.f, 1.f, sustain);
    o.releaseMs = juce::jlimit(1.f, 4000.f, releaseMs);
    o.updateEnvCoeffs();
}

void FmVoice::setGlideMs(float ms)
{
    glideMs = juce::jmax(0.f, ms);
    glideCoef = glideMs > 0.1f ? std::exp(-1.f / (glideMs * 0.001f * sampleRate)) : 0.f;
}

float FmVoice::midiNoteToHz(float note) const noexcept
{
    return 440.f * std::pow(2.f, (note - 69.f) / 12.f);
}

void FmVoice::updatePhaseIncrements()
{
    if (glideCoef > 0.f)
    {
        glideCurrentNote += (1.f - glideCoef) * (glideTargetNote - glideCurrentNote);
        if (std::abs(glideCurrentNote - glideTargetNote) < 0.001f)
            glideCurrentNote = glideTargetNote;
    }
    else
        glideCurrentNote = glideTargetNote;

    const float hz = midiNoteToHz(glideCurrentNote);
    const float w = hz * juce::MathConstants<float>::twoPi / sampleRate;
    for (int i = 0; i < kNumOps; ++i)
        ops[(size_t) i].phaseInc = w;
}

void FmVoice::noteOn(int midiNote, float velocity)
{
    active = true;
    glideTargetNote = (float) midiNote;
    noteVelocity = juce::jlimit(0.01f, 1.f, velocity);
    if (glideMs < 0.1f)
        glideCurrentNote = glideTargetNote;

    for (auto& o : ops)
        o.noteOn();
}

void FmVoice::noteOff()
{
    for (auto& o : ops)
        o.noteOff();
}

float FmVoice::quietMetric() const noexcept
{
    float m = 1.f;
    for (const auto& o : ops)
        m = std::min(m, o.envValue);
    return m;
}

float FmVoice::process()
{
    if (!active)
        return 0.f;

    updatePhaseIncrements();

    const float md = kModDepth;
    float out = 0.f;

    switch (algorithm)
    {
        case 0: // 4→3→2→1
        {
            const float m3 = ops[3].tick(0.f);
            const float m2 = ops[2].tick(m3 * md);
            const float m1 = ops[1].tick(m2 * md);
            out = ops[0].tick(m1 * md);
            break;
        }
        case 1: // [4,3]→2→1
        {
            const float p = (ops[3].tick(0.f) + ops[2].tick(0.f)) * 0.5f;
            const float m1 = ops[1].tick(p * md);
            out = ops[0].tick(m1 * md);
            break;
        }
        case 2: // dual chain
        {
            out = ops[0].tick(ops[1].tick(0.f) * md) + ops[2].tick(ops[3].tick(0.f) * md);
            out *= 0.5f;
            break;
        }
        case 3: // [4,3,2]→1
        {
            const float p = (ops[3].tick(0.f) + ops[2].tick(0.f) + ops[1].tick(0.f)) * (1.f / 3.f);
            out = ops[0].tick(p * md);
            break;
        }
        case 4: // parallel
        default:
            out = ops[0].tick(0.f) + ops[1].tick(0.f) + ops[2].tick(0.f) + ops[3].tick(0.f);
            out *= 0.25f;
            break;
    }

    const float velScale = 0.15f + 0.85f * noteVelocity;
    out *= velScale;

    bool anyEnv = false;
    for (const auto& o : ops)
        if (o.stage != FmOperator::Stage::Idle)
        {
            anyEnv = true;
            break;
        }
    if (!anyEnv)
        active = false;

    return out;
}
