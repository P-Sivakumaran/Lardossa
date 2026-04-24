#pragma once

#include "TransportTypes.h"

namespace lardossa::transport
{
BlockTransport resolveBlock(MutableState& st,
                            juce::AudioProcessor::WrapperType wrapperType,
                            juce::AudioPlayHead* playHead,
                            double sampleRate,
                            int numSamples,
                            bool standalonePlay,
                            double standaloneBpm);
} // namespace lardossa::transport
