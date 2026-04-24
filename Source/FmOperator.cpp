#include "FmOperator.h"
#include <algorithm>

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.f * kPi;

inline float expCoeff(float timeMs, float sr)
{
    if (timeMs <= 0.1f)
        return 1.f;
    return 1.f - std::exp(-1.f / (timeMs * 0.001f * sr));
}
} // namespace

void FmOperator::setSampleRate(float sr)
{
    sampleRate = std::max(100.f, sr);
    updateEnvCoeffs();
}

void FmOperator::updateEnvCoeffs()
{
    attackCoeff = expCoeff(attackMs, sampleRate);
    decayCoeff = expCoeff(decayMs, sampleRate);
    releaseCoeff = expCoeff(releaseMs, sampleRate);
}

void FmOperator::noteOn()
{
    if (stage == Stage::Idle)
        envValue = 0.f;
    stage = Stage::Attack;
}

void FmOperator::noteOff()
{
    if (stage != Stage::Idle)
        stage = Stage::Release;
}

float FmOperator::tickEnv()
{
    switch (stage)
    {
        case Stage::Idle:
            envValue = 0.f;
            break;
        case Stage::Attack:
            envValue += (1.f - envValue) * attackCoeff;
            if (envValue > 0.999f)
            {
                envValue = 1.f;
                stage = Stage::Decay;
            }
            break;
        case Stage::Decay:
            envValue += (sustainLevel - envValue) * decayCoeff;
            if (std::abs(envValue - sustainLevel) < 0.001f)
            {
                envValue = sustainLevel;
                stage = Stage::Sustain;
            }
            break;
        case Stage::Sustain:
            break;
        case Stage::Release:
            envValue += (0.f - envValue) * releaseCoeff;
            if (envValue < 1.0e-5f)
            {
                envValue = 0.f;
                stage = Stage::Idle;
            }
            break;
    }
    return envValue;
}

float FmOperator::tick(float modIn)
{
    const float env = tickEnv();
    const float detuneMul = std::pow(2.f, detuneSemis / 12.f);
    const float w = phaseInc * detuneMul * ratio;

    const float fb = feedback * 0.35f * (fbState[0] + fbState[1]) * 0.5f;
    fbState[1] = fbState[0];
    phase += w + modIn + fb;
    while (phase > kTwoPi)
        phase -= kTwoPi;
    while (phase < 0.f)
        phase += kTwoPi;

    const float s = std::sin(phase) * outputLevel * env;
    fbState[0] = s;
    return s;
}
