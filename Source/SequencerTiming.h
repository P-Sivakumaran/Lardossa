#pragma once

#include "SequencerTypes.h"

namespace seq
{
float normaliseStepLength(float x);
double patternLengthSixteenths(int seqLen, const SequencerStepArray& steps);
int stepIndexFromPatternPpq(double ppq, int seqLen, const SequencerStepArray& steps);
} // namespace seq
