#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <deque>
#include <optional>
#include <random>

#include "SequencerTypes.h"
#include "TransportTypes.h"
#include "VoicePool.h"

class LardossaAudioProcessor final : public juce::AudioProcessor
{
public:
    static constexpr int kStateVersion = 1;

    LardossaAudioProcessor();
    ~LardossaAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    LardossaVoicePool& getVoicePool() noexcept { return voicePool; }

    void handleWebNativeEvent(const juce::String& eventType, const juce::var& payload);

    int getSequencerLength() const;
    void replaceSequencerStepFromWeb(int stepIndex, int note, float velocity, bool accent, bool slide, bool tie,
                                     float lengthInSixteenths, bool enabled);
    void setSequencerAccentFromWeb(int stepIndex, bool accent);
    void setSequencerSlideFromWeb(int stepIndex, bool slide);
    void setSequencerTieFromWeb(int stepIndex, bool tie);

    void applyFactoryPresetByName(const juce::String& name);
    juce::StringArray getFactoryPresetNames() const;

    juce::String getUiStateJson();

private:
    using TransportContext = lardossa::transport::BlockTransport;

    TransportContext resolveTransportContext(int numSamples);
    void updateVoiceParamsFromApvts();
    const SequencerStepArray& snapshotSequencerStepsForAudioThread();
    void processSequencerVoice(juce::AudioBuffer<float>& buffer, float* monoFmScratch, const TransportContext& transport,
                               const SequencerStepArray& stepsCopy, bool fmEngine);
    std::optional<double> applySequencerStepChange(int prevIdx, int currentStep, double stepOnsetPpq,
                                                   const TransportContext& transport, const SequencerStepArray& stepsCopy,
                                                   bool fmEngine);
    void processExternalMidi(const juce::MidiBuffer& midiMessages, bool seqRecordArmed, bool fmEngine);
    void saveSequencerStepsToState(juce::ValueTree& state) const;
    void restoreSequencerStepsFromState(const juce::ValueTree& state);

    juce::UndoManager apvtsUndoManager;
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    LardossaVoicePool voicePool{};
    lardossa::transport::MutableState transportState_{};
    double lastKnownBpm = 120.0;
    mutable juce::SpinLock sequencerLock;
    SequencerStepArray sequencerSteps{};
    SequencerStepArray sequencerStepsAudioCache{};
    std::atomic<int> lastStepIndex{-1};
    int lastSequencerNote = -1;
    uint64_t midiNoteCounter = 0;
    std::optional<double> seqGateEndPpq;
    bool seqVoiceRenderedThisBlock = false;

    std::mt19937 rng{std::random_device{}()};

    std::atomic<bool> panicRequested{false};

    std::atomic<float>* p_engineMode = nullptr;
    std::atomic<float>* p_algorithm = nullptr;
    std::atomic<float>* p_op1Ratio = nullptr;
    std::atomic<float>* p_op2Ratio = nullptr;
    std::atomic<float>* p_op3Ratio = nullptr;
    std::atomic<float>* p_op4Ratio = nullptr;
    std::atomic<float>* p_op1Level = nullptr;
    std::atomic<float>* p_op2Level = nullptr;
    std::atomic<float>* p_op3Level = nullptr;
    std::atomic<float>* p_op4Level = nullptr;
    std::atomic<float>* p_op1Feedback = nullptr;
    std::atomic<float>* p_voltage = nullptr;
    std::atomic<float>* p_depth = nullptr;
    std::atomic<float>* p_pressure = nullptr;
    std::atomic<float>* p_decay = nullptr;
    std::atomic<float>* p_ampAttackMs = nullptr;
    std::atomic<float>* p_ampDecayMs = nullptr;
    std::atomic<float>* p_ampSustain = nullptr;
    std::atomic<float>* p_ampReleaseMs = nullptr;
    std::atomic<float>* p_glideMs = nullptr;
    std::atomic<float>* p_oscDetune = nullptr;
    std::atomic<float>* p_seqLength = nullptr;
    std::atomic<float>* p_seqRecord = nullptr;
    std::atomic<float>* p_standalonePlay = nullptr;
    std::atomic<float>* p_standaloneBpm = nullptr;
    std::atomic<float>* p_seqVariety = nullptr;
    std::atomic<float>* p_outputGain = nullptr;
    std::atomic<float>* p_filterDrive = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LardossaAudioProcessor)
};
