#pragma once

#include <JuceHeader.h>

namespace lardossa::transport
{
struct BlockTransport
{
    bool isPlaying = false;
    double ppqPosition = 0.0;
    double bpmForBlock = 120.0;
    double sampleRate = 44100.0;
    int numSamples = 0;
};

struct MutableState
{
    double standalonePpqPosition = 0.0;
    bool standaloneWasPlaying = false;
    double hostFallbackPpqPosition = 0.0;
    bool hostWasPlaying = false;
};
} // namespace lardossa::transport
