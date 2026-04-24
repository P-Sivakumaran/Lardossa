#include <catch2/catch_test_macros.hpp>

#include <LardossaPresets.h>
#include <PresetManager.h>

TEST_CASE("readParamMapFromPresetXml parses param elements", "[preset]")
{
    juce::StringPairArray m;
    const juce::String xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<LardossaPreset version="1" name="Test">
  <param id="voltage" value="180"/>
  <param id="algorithm" value="2"/>
</LardossaPreset>)";

    REQUIRE(PresetManager::readParamMapFromPresetXml(xml, m));
    REQUIRE(m["voltage"] == "180");
    REQUIRE(m["algorithm"] == "2");
}

TEST_CASE("embedded LARDOSSEN BASS preset bytes parse", "[preset]")
{
    juce::StringPairArray m;
    const juce::String xml = juce::String::fromUTF8(lardossa_presets::LARDOSSEN_BASS_xml,
                                                     lardossa_presets::LARDOSSEN_BASS_xmlSize);
    REQUIRE(PresetManager::readParamMapFromPresetXml(xml, m));
    REQUIRE(m["voltage"] == "180");
    REQUIRE(m["algorithm"] == "1");
}
