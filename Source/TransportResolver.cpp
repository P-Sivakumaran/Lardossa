#include "TransportResolver.h"

namespace lardossa::transport
{
BlockTransport resolveBlock(MutableState& st,
                            juce::AudioProcessor::WrapperType wrapperType,
                            juce::AudioPlayHead* playHead,
                            double sampleRate,
                            int numSamples,
                            bool standalonePlay,
                            double standaloneBpm)
{
    BlockTransport ctx;
    ctx.numSamples = numSamples;
    ctx.sampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;

    bool hostTransportAvailable = false;
    bool hostProvidedPpq = false;
    ctx.bpmForBlock = standaloneBpm;

    if (wrapperType != juce::AudioProcessor::wrapperType_Standalone)
    {
        if (playHead != nullptr)
        {
            if (const auto pos = playHead->getPosition())
            {
                hostTransportAvailable = true;
                if (pos->getBpm().hasValue())
                    ctx.bpmForBlock = *pos->getBpm();
                if (pos->getPpqPosition().hasValue())
                {
                    ctx.ppqPosition = *pos->getPpqPosition();
                    hostProvidedPpq = true;
                }
                ctx.isPlaying = pos->getIsPlaying();
            }
        }
    }

    const auto numSamp = (double) numSamples;
    if (hostTransportAvailable && ctx.isPlaying)
    {
        if (hostProvidedPpq)
        {
            st.hostFallbackPpqPosition = ctx.ppqPosition;
            st.hostWasPlaying = true;
        }
        else
        {
            ctx.isPlaying = false;
            st.hostWasPlaying = false;
            st.hostFallbackPpqPosition = 0.0;
        }
    }
    else
    {
        st.hostWasPlaying = false;
        if (standalonePlay)
        {
            if (!st.standaloneWasPlaying)
                st.standalonePpqPosition = 0.0;

            st.standalonePpqPosition += (numSamp / ctx.sampleRate) * (standaloneBpm / 60.0);
            ctx.isPlaying = true;
            ctx.ppqPosition = st.standalonePpqPosition;
            ctx.bpmForBlock = standaloneBpm;
        }

        st.standaloneWasPlaying = standalonePlay;
    }

    return ctx;
}
} // namespace lardossa::transport
