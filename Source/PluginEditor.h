#pragma once

#include <JuceHeader.h>

class LardossaAudioProcessor;

class LardossaAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit LardossaAudioProcessorEditor(LardossaAudioProcessor&);
    ~LardossaAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void pushStateToUI();

private:
    LardossaAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> browser;
    bool pageReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LardossaAudioProcessorEditor)
};
