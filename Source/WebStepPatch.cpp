#include "WebStepPatch.h"
#include "PluginProcessor.h"
#include "SequencerTypes.h"

namespace lardossa::sequencer
{
namespace
{
bool asBool(const juce::var& v) noexcept { return (bool) v; }

int asMidiNote(const juce::var& v) noexcept
{
    const double x = (double) v;
    if (!std::isfinite(x))
        return 57;
    return juce::jlimit(0, 127, (int) std::lround(x));
}

float asVelocity(const juce::var& v) noexcept
{
    const double x = (double) v;
    if (!std::isfinite(x))
        return 0.85f;
    return juce::jlimit(0.01f, 1.0f, (float) x);
}

float asLength(const juce::var& v) noexcept
{
    const double x = (double) v;
    if (!std::isfinite(x))
        return 1.0f;
    return (float) x;
}

int asStepIndex(const juce::var& d) noexcept
{
    const double x = (double) d["index"];
    if (!std::isfinite(x))
        return -1;
    return (int) std::lround(x);
}
} // namespace

void applyWebStep(LardossaAudioProcessor& proc, const juce::var& d)
{
    const int idx = asStepIndex(d);
    if (!juce::isPositiveAndBelow(idx, kNumSequencerSteps))
        return;

    proc.replaceSequencerStepFromWeb(idx,
                                     asMidiNote(d["note"]),
                                     asVelocity(d["vel"]),
                                     asBool(d["accent"]),
                                     asBool(d["slide"]),
                                     asBool(d["tie"]),
                                     asLength(d["len"]),
                                     asBool(d["on"]));
}

void applyWebStepAccent(LardossaAudioProcessor& proc, const juce::var& d)
{
    const int idx = asStepIndex(d);
    if (!juce::isPositiveAndBelow(idx, kNumSequencerSteps))
        return;
    proc.setSequencerAccentFromWeb(idx, asBool(d["value"]));
}

void applyWebStepSlide(LardossaAudioProcessor& proc, const juce::var& d)
{
    const int idx = asStepIndex(d);
    if (!juce::isPositiveAndBelow(idx, kNumSequencerSteps))
        return;
    proc.setSequencerSlideFromWeb(idx, asBool(d["value"]));
}

void applyWebStepTie(LardossaAudioProcessor& proc, const juce::var& d)
{
    const int idx = asStepIndex(d);
    if (!juce::isPositiveAndBelow(idx, kNumSequencerSteps))
        return;
    proc.setSequencerTieFromWeb(idx, asBool(d["value"]));
}
} // namespace lardossa::sequencer
