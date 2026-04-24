#pragma once

#include "FmVoice.h"
#include "SubtractiveVoice.h"
#include <array>
#include <cstdint>

/** Eight voices; FM or subtractive engine per global mode. */
class LardossaVoicePool
{
public:
    static constexpr int kNumVoices = 8;
    static constexpr int kSeqVoiceSlot = 0;

    void prepare(double sampleRate, int maxBlockSize);

    /** @return voice index used */
    int noteOn(int midiNote, float velocity, bool fmEngine);
    void noteOff(int midiNote, bool fmEngine);
    void panic();

    void setGlideAll(float glideMs, bool fmEngine);

    void updateFmVoice(int idx, int algorithm, const float* opRatio, const float* opDetune,
                       const float* opLevel, float op1Fb,
                       const float* opAtk, const float* opDec, const float* opSus, const float* opRel);

    void updateSubVoice(int idx, float oscDetuneCents, float voltage, float depth, float pressure, float decay,
                        float ampA, float ampD, float ampS, float ampR, float filterDrive, float glideMs);

    void renderFmVoiceAdding(int voiceIdx, float* monoInterleaved, int destStart, int numSamples);
    void renderSubVoiceAdding(int voiceIdx, juce::AudioBuffer<float>& buffer, int destStart, int numSamples);

    void startSeqNoteFm(int midi, float vel, bool accent, bool legato, bool slide);
    void startSeqNoteSub(int midi, float vel, bool accent, bool legato, bool slide);
    void stopSeqVoiceFm();
    void stopSeqVoiceSub();

    int getSlotMidi(int i) const noexcept { return currentNote[(size_t) i]; }
    uint64_t getSlotAge(int i) const noexcept { return noteAge[(size_t) i]; }

    float fmQuietMetric(int i) const noexcept { return fm[(size_t) i].quietMetric(); }

private:
    int findFreeSlot() const noexcept;
    int findStealSlotFm() const noexcept;
    int findStealSlotSub() const noexcept;

    std::array<FmVoice, kNumVoices> fm{};
    std::array<SubtractiveVoice, kNumVoices> sub{};
    std::array<int, kNumVoices> currentNote{};
    std::array<uint64_t, kNumVoices> noteAge{};
    uint64_t ageCounter = 0;
    double sr = 44100.0;
    int preparedBlock = 512;
};
