#pragma once

#include <cmath>

/** Single FM operator: sine oscillator + per-operator ADSR. */
struct FmOperator
{
    float phase = 0.f;
    float phaseInc = 0.f;
    float ratio = 1.f;
    float detuneSemis = 0.f;
    float outputLevel = 1.f;
    float feedback = 0.f;
    float fbState[2] = {};

    enum class Stage
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };
    Stage stage = Stage::Idle;
    float envValue = 0.f;
    float attackMs = 2.f;
    float decayMs = 100.f;
    float sustainLevel = 0.8f;
    float releaseMs = 200.f;
    float sampleRate = 44100.f;

    float attackCoeff = 0.f;
    float decayCoeff = 0.f;
    float releaseCoeff = 0.f;

    void noteOn();
    void noteOff();
    void setSampleRate(float sr);
    void updateEnvCoeffs();

    /** Carrier/modulator sample; advances phase. `modIn` is PM radians. */
    float tick(float modIn);

    /** Advance envelope only (used when op is mod-only path). */
    float tickEnv();
};
