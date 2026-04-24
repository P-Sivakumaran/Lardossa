#pragma once

#include <JuceHeader.h>

class LardossaAudioProcessor;

namespace lardossa::sequencer
{
void applyWebStep(LardossaAudioProcessor& proc, const juce::var& d);
void applyWebStepAccent(LardossaAudioProcessor& proc, const juce::var& d);
void applyWebStepSlide(LardossaAudioProcessor& proc, const juce::var& d);
void applyWebStepTie(LardossaAudioProcessor& proc, const juce::var& d);
} // namespace lardossa::sequencer
