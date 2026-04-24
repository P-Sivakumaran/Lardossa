#pragma once

#include <juce_core/juce_core.h>

namespace lardossa::dsp
{
inline float fastTanh(float x) noexcept
{
    const float x2 = x * x;
    return juce::jlimit(-1.0f, 1.0f, x * (27.0f + x2) / (27.0f + 9.0f * x2));
}
} // namespace lardossa::dsp
