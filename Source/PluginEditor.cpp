#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <LardossaUiData.h>
#include <memory>

static std::optional<juce::WebBrowserComponent::Resource> serveUiResource(const juce::String& url)
{
    using R = juce::WebBrowserComponent::Resource;
    auto make = [](const char* data, int size, const char* mime) -> std::optional<R>
    {
        return R{std::vector<std::byte>(reinterpret_cast<const std::byte*>(data),
                                         reinterpret_cast<const std::byte*>(data) + size),
                   mime};
    };

    if (url == "/" || url == "/index.html")
        return make(lardossa_ui::index_html, lardossa_ui::index_htmlSize, "text/html");
    if (url.endsWith("/assets/index.js") || url.endsWith("assets/index.js"))
        return make(lardossa_ui::index_js, lardossa_ui::index_jsSize, "text/javascript");
    if (url.endsWith("/assets/index.css") || url.endsWith("assets/index.css"))
        return make(lardossa_ui::index_css, lardossa_ui::index_cssSize, "text/css");
    return std::nullopt;
}

LardossaAudioProcessorEditor::LardossaAudioProcessorEditor(LardossaAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::Component::SafePointer<LardossaAudioProcessorEditor> safeThis(this);

    using WV2 = juce::WebBrowserComponent::Options::WinWebView2;
    auto opts = juce::WebBrowserComponent::Options{}
                    .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                    .withNativeIntegrationEnabled()
                    .withResourceProvider(serveUiResource)
                    .withWinWebView2Options(WV2{}.withBackgroundColour(juce::Colour(0xff0a0a0a)))

                    .withEventListener("pageReady", [safeThis](const juce::var&)
                                         {
                                             if (safeThis != nullptr)
                                             {
                                                 safeThis->pageReady = true;
                                                 safeThis->pushStateToUI();
                                             }
                                         })
                    .withEventListener("paramChange", [&proc = audioProcessor](const juce::var& d)
                                         { proc.handleWebNativeEvent("paramChange", d); })
                    .withEventListener("panicAll", [&proc = audioProcessor](const juce::var&)
                                         { proc.handleWebNativeEvent("panicAll", {}); })
                    .withEventListener("seqStep", [&proc = audioProcessor](const juce::var& d)
                                         { proc.handleWebNativeEvent("seqStep", d); })
                    .withEventListener("setStep", [&proc = audioProcessor](const juce::var& d)
                                         { proc.handleWebNativeEvent("setStep", d); })
                    .withEventListener("generatePattern", [&proc = audioProcessor](const juce::var& d)
                                         { proc.handleWebNativeEvent("generatePattern", d); })
                    .withEventListener("loadFactoryPreset", [&proc = audioProcessor](const juce::var& d)
                                         { proc.handleWebNativeEvent("loadFactoryPreset", d); });

    browser = std::make_unique<juce::WebBrowserComponent>(opts);
    addAndMakeVisible(*browser);

    setOpaque(true);
    setSize(900, 560);
    setResizable(false, false);
    startTimerHz(30);

    browser->goToURL(juce::WebBrowserComponent::getResourceProviderRoot() + "index.html");
}

LardossaAudioProcessorEditor::~LardossaAudioProcessorEditor()
{
    stopTimer();
    if (browser != nullptr)
        browser.reset();
}

void LardossaAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0a0a0a));
}

void LardossaAudioProcessorEditor::resized()
{
    if (browser != nullptr)
        browser->setBounds(getLocalBounds());
}

void LardossaAudioProcessorEditor::timerCallback()
{
    if (pageReady)
        pushStateToUI();
}

void LardossaAudioProcessorEditor::pushStateToUI()
{
    if (browser == nullptr)
        return;
    const auto json = audioProcessor.getUiStateJson();
    browser->emitEventIfBrowserIsVisible("stateUpdate", json);
}
