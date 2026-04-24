#include "PresetManager.h"
#include <LardossaPresets.h>

namespace
{
const char* getPresetXmlData(const juce::String& name, int& outSize)
{
    outSize = 0;
    const auto n = name.trim();

    if (n == "LARDOSSEN BASS")
    {
        outSize = lardossa_presets::LARDOSSEN_BASS_xmlSize;
        return lardossa_presets::LARDOSSEN_BASS_xml;
    }
    if (n == "POSITRON STAB")
    {
        outSize = lardossa_presets::POSITRON_STAB_xmlSize;
        return lardossa_presets::POSITRON_STAB_xml;
    }
    if (n == "AQUABAHN LEAD")
    {
        outSize = lardossa_presets::AQUABAHN_LEAD_xmlSize;
        return lardossa_presets::AQUABAHN_LEAD_xml;
    }
    if (n == "GRAVA PAD")
    {
        outSize = lardossa_presets::GRAVA_PAD_xmlSize;
        return lardossa_presets::GRAVA_PAD_xml;
    }
    if (n == "LARDOSSA FLOOD")
    {
        outSize = lardossa_presets::LARDOSSA_FLOOD_xmlSize;
        return lardossa_presets::LARDOSSA_FLOOD_xml;
    }

    return nullptr;
}
} // namespace

juce::StringArray PresetManager::getFactoryPresetNames()
{
    return {"LARDOSSEN BASS", "POSITRON STAB", "AQUABAHN LEAD", "GRAVA PAD", "LARDOSSA FLOOD"};
}

bool PresetManager::readParamMapFromPresetXml(const juce::String& xmlText, juce::StringPairArray& outIdToValueString)
{
    outIdToValueString.clear();
    juce::XmlDocument doc(xmlText);
    if (auto root = doc.getDocumentElement())
    {
        for (auto* e = root->getFirstChildElement(); e != nullptr; e = e->getNextElement())
        {
            if (e->hasTagName("param"))
            {
                const auto id = e->getStringAttribute("id");
                const auto val = e->getStringAttribute("value");
                if (id.isNotEmpty())
                    outIdToValueString.set(id, val);
            }
        }
        return outIdToValueString.size() > 0;
    }
    return false;
}

bool PresetManager::applyParamMap(juce::AudioProcessorValueTreeState& apvts, const juce::StringPairArray& map)
{
    for (int i = 0; i < map.size(); ++i)
    {
        const auto id = map.getAllKeys()[i];
        const auto valStr = map.getAllValues()[i];
        auto* param = apvts.getParameter(id);
        if (param == nullptr)
            continue;

        if (auto* pb = dynamic_cast<juce::AudioParameterBool*>(param))
        {
            const bool on = valStr.getIntValue() != 0 || valStr.trim().equalsIgnoreCase("true");
            *pb = on;
            continue;
        }

        if (auto* pi = dynamic_cast<juce::AudioParameterInt*>(param))
        {
            *pi = valStr.getIntValue();
            continue;
        }

        if (auto* pc = dynamic_cast<juce::AudioParameterChoice*>(param))
        {
            const int idx = juce::jlimit(0, pc->choices.size() - 1, valStr.getIntValue());
            *pc = idx;
            continue;
        }

        const float v = valStr.getFloatValue();
        param->setValueNotifyingHost((float) param->convertTo0to1((double) v));
    }

    return true;
}

bool PresetManager::applyFactoryPreset(juce::AudioProcessorValueTreeState& apvts, const juce::String& presetName)
{
    int sz = 0;
    const char* data = getPresetXmlData(presetName, sz);
    if (data == nullptr || sz <= 0)
        return false;

    const juce::String xml(data, sz);
    juce::StringPairArray map;
    if (!readParamMapFromPresetXml(xml, map))
        return false;

    return applyParamMap(apvts, map);
}
