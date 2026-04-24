#pragma once

#include "SequencerTypes.h"
#include <random>

namespace patterns
{
void clearSequencer(SequencerStepArray& steps);
void generateEbmBassPattern(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety);

void generateDetroitBass(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety);
void generateDetroitLead(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety);
void generateElectroGroove(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety);
} // namespace patterns
