#pragma once

#include <array>

struct SequencerStep
{
    int note = 57;
    float velocity = 0.85f;
    float lengthFrac = 1.0f;
    bool enabled = false;
    bool accent = false;
    bool slide = false;
    bool tie = false;
};

inline constexpr int kNumSequencerSteps = 32;
using SequencerStepArray = std::array<SequencerStep, kNumSequencerSteps>;
