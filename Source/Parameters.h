#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

inline juce::AudioProcessorValueTreeState::ParameterLayout createLardossaParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "engineMode", "Engine", juce::StringArray{"Subtractive", "FM"}, 1));

    params.push_back(std::make_unique<juce::AudioParameterInt>("algorithm", "Algorithm", 1, 5, 1,
                                                                 juce::AudioParameterIntAttributes()
                                                                     .withStringFromValueFunction(
                                                                         [](int v, int) { return "ALGO " + juce::String(v); })
                                                                     .withValueFromStringFunction(
                                                                         [](const juce::String& t)
                                                                         {
                                                                             return t.getTrailingIntValue();
                                                                         })));

    auto ratioRange = juce::NormalisableRange<float>(0.5f, 8.0f, 0.01f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op1Ratio", "Op1 Ratio", ratioRange, 1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) { return juce::String::formatted("x%.2f", v); })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op2Ratio", "Op2 Ratio", ratioRange, 2.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) { return juce::String::formatted("x%.2f", v); })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op3Ratio", "Op3 Ratio", ratioRange, 3.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) { return juce::String::formatted("x%.2f", v); })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op4Ratio", "Op4 Ratio", ratioRange, 7.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float v, int) { return juce::String::formatted("x%.2f", v); })));

    auto pct = [](float v, int)
    {
        return juce::String::formatted("%.0f%%", (double) v * 100.0);
    };
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op1Level", "Op1 Level", juce::NormalisableRange<float>(0.f, 1.f), 1.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op2Level", "Op2 Level", juce::NormalisableRange<float>(0.f, 1.f), 0.8f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op3Level", "Op3 Level", juce::NormalisableRange<float>(0.f, 1.f), 0.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op4Level", "Op4 Level", juce::NormalisableRange<float>(0.f, 1.f), 0.3f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "op1Feedback", "Op1 Feedback", juce::NormalisableRange<float>(0.f, 1.f), 0.f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "voltage", "Voltage", juce::NormalisableRange<float>(20.f, 20000.f, 0.01f, 0.3f), 800.f,
        juce::AudioParameterFloatAttributes().withLabel("Hz").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " Hz"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "depth", "Depth", juce::NormalisableRange<float>(0.f, 1.f), 0.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "pressure", "Pressure", juce::NormalisableRange<float>(0.f, 1.f), 0.5f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay", juce::NormalisableRange<float>(1.f, 2000.f, 0.01f, 0.35f), 200.f,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " ms"; })));

    auto msSkew = [](float lo, float hi)
    {
        return juce::NormalisableRange<float>(lo, hi, 0.01f, 0.35f);
    };

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ampAttackMs", "Amp Attack", msSkew(0.5f, 2000.f), 2.f,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 2) + " ms"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ampDecayMs", "Amp Decay", msSkew(1.f, 2000.f), 100.f,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " ms"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ampSustain", "Amp Sustain", juce::NormalisableRange<float>(0.f, 1.f), 0.7f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ampReleaseMs", "Amp Release", msSkew(1.f, 2000.f), 200.f,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " ms"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "glideMs", "Glide", msSkew(0.f, 500.f), 0.f,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " ms"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "oscDetune", "Osc Detune", juce::NormalisableRange<float>(-100.f, 100.f, 0.01f), 0.f,
        juce::AudioParameterFloatAttributes().withLabel("cents").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " ct"; })));

    params.push_back(std::make_unique<juce::AudioParameterInt>("seqLength", "Seq Length", 1, 32, 16));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "seqRecord", "Seq Record", false, juce::AudioParameterBoolAttributes().withAutomatable(false)));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "standalonePlay", "Standalone Play", false, juce::AudioParameterBoolAttributes().withAutomatable(false)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "standaloneBpm", "Standalone BPM", juce::NormalisableRange<float>(60.f, 200.f, 0.01f), 120.f,
        juce::AudioParameterFloatAttributes().withLabel("BPM").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " BPM"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "seqVariety", "Seq Variety", juce::NormalisableRange<float>(0.f, 1.f), 0.5f,
        juce::AudioParameterFloatAttributes().withAutomatable(false)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain", "Output Gain", juce::NormalisableRange<float>(-24.f, 6.f, 0.01f), 0.f,
        juce::AudioParameterFloatAttributes().withLabel("dB").withStringFromValueFunction(
            [](float v, int) { return juce::String(v, 1) + " dB"; })));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterDrive", "Filter Drive", juce::NormalisableRange<float>(0.f, 1.f), 0.2f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(pct)));

    return {params.begin(), params.end()};
}
