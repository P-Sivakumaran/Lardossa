#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/** Factory presets: XML in `Presets/Factory/` embedded as `LardossaPresets` BinaryData. */
class PresetManager
{
public:
    static juce::StringArray getFactoryPresetNames();

    /** Apply embedded factory preset to APVTS (real units in XML). */
    static bool applyFactoryPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& presetName);

    /** Parse `<param id="" value=""/>` elements under `<LardossaPreset>`. */
    static bool readParamMapFromPresetXml(const juce::String& xmlText, juce::StringPairArray& outIdToValueString);

private:
    static bool applyParamMap(juce::AudioProcessorValueTreeState& apvts, const juce::StringPairArray& map);
};
