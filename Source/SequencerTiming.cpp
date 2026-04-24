#include "SequencerTiming.h"
#include <juce_core/juce_core.h>
#include <cmath>

namespace seq
{

float normaliseStepLength(float x)
{
    if (x <= 0.375f)
        return 0.25f;
    if (x <= 0.75f)
        return 0.5f;
    if (x <= 1.5f)
        return 1.0f;
    if (x <= 3.0f)
        return 2.0f;
    return 4.0f;
}

double patternLengthSixteenths(int seqLen, const SequencerStepArray& steps)
{
    double sum = 0.0;
    for (int i = 0; i < seqLen; ++i)
        sum += (double) juce::jmax(0.25f, normaliseStepLength(steps[(size_t) i].lengthFrac));
    return sum;
}

int stepIndexFromPatternPpq(double ppq, int seqLen, const SequencerStepArray& steps)
{
    float lengths[kNumSequencerSteps];
    double pat = 0.0;
    for (int i = 0; i < seqLen; ++i)
    {
        lengths[i] = juce::jmax(0.25f, normaliseStepLength(steps[(size_t) i].lengthFrac));
        pat += (double) lengths[i];
    }

    if (pat <= 0.0)
        return 0;

    double w = std::fmod(ppq * 4.0, pat);
    if (w < 0.0)
        w += pat;

    double acc = 0.0;
    for (int i = 0; i < seqLen; ++i)
    {
        if (w < acc + (double) lengths[i])
            return i;
        acc += (double) lengths[i];
    }

    return juce::jmax(0, seqLen - 1);
}

} // namespace seq
