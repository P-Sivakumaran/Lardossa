#pragma once

#include "FmOperator.h"

/** Four-operator FM voice; single-sample `process()` — no heap on audio thread. */
class FmVoice
{
public:
    static constexpr int kNumOps = 4;
    static constexpr float kModDepth = 3.5f;

    enum class Algorithm
    {
        Algo1 = 0,
        Algo2,
        Algo3,
        Algo4,
        Algo5,
    };

    void noteOn(int midiNote, float velocity);
    void noteOff();

    float process();

    void setSampleRate(float sr);
    void setAlgorithm(int algoIndex);

    void setOperatorParams(int opIdx, float ratio, float detune, float level, float feedback);
    void setOperatorEnv(int opIdx, float attackMs, float decayMs, float sustain, float releaseMs);

    void setGlideMs(float glideMs);
    bool isActive() const noexcept { return active; }
    /** Lowest envelope value across operators — for voice stealing. */
    float quietMetric() const noexcept;

private:
    void updatePhaseIncrements();
    float midiNoteToHz(float note) const noexcept;

    FmOperator ops[kNumOps]{};
    float sampleRate = 44100.f;
    int algorithm = 0;
    float glideMs = 0.f;
    float glideCoef = 0.f;
    float glideCurrentNote = 60.f;
    float glideTargetNote = 60.f;
    float noteVelocity = 1.f;
    bool active = false;
};
