#include "VoicePool.h"
#include <limits>

void LardossaVoicePool::prepare(double sampleRate, int maxBlockSize)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    preparedBlock = juce::jmax(1, maxBlockSize);
    for (auto& v : fm)
        v.setSampleRate((float) sr);
    for (auto& v : sub)
        v.setSampleRate(sr, preparedBlock);
}

int LardossaVoicePool::findFreeSlot() const noexcept
{
    for (int i = 1; i < kNumVoices; ++i)
        if (!fm[(size_t) i].isActive() && !sub[(size_t) i].isActive())
            return i;
    return -1;
}

int LardossaVoicePool::findStealSlotFm() const noexcept
{
    int best = 1;
    float bestQ = std::numeric_limits<float>::max();
    for (int i = 1; i < kNumVoices; ++i)
    {
        if (!fm[(size_t) i].isActive())
            return i;
        const float q = fm[(size_t) i].quietMetric();
        if (q < bestQ)
        {
            bestQ = q;
            best = i;
        }
    }
    return best;
}

int LardossaVoicePool::findStealSlotSub() const noexcept
{
    int best = 1;
    uint64_t youngest = std::numeric_limits<uint64_t>::max();
    for (int i = 1; i < kNumVoices; ++i)
    {
        if (!sub[(size_t) i].isActive())
            return i;
        if (noteAge[(size_t) i] < youngest)
        {
            youngest = noteAge[(size_t) i];
            best = i;
        }
    }
    return best;
}

int LardossaVoicePool::noteOn(int midiNote, float velocity, bool fmEngine)
{
    int slot = findFreeSlot();
    if (slot < 0)
    {
        slot = fmEngine ? findStealSlotFm() : findStealSlotSub();
        if (fmEngine)
            fm[(size_t) slot].noteOff();
        else
            sub[(size_t) slot].stopNote();
    }

    if (fmEngine)
    {
        fm[(size_t) slot].noteOn(midiNote, velocity);
        sub[(size_t) slot].stopNote();
    }
    else
    {
        sub[(size_t) slot].startNote(midiNote, velocity, false, false, false);
        fm[(size_t) slot].noteOff();
    }

    currentNote[(size_t) slot] = midiNote;
    noteAge[(size_t) slot] = ++ageCounter;
    return slot;
}

void LardossaVoicePool::noteOff(int midiNote, bool fmEngine)
{
    for (int i = 1; i < kNumVoices; ++i)
    {
        if (currentNote[(size_t) i] == midiNote)
        {
            if (fmEngine)
                fm[(size_t) i].noteOff();
            else
                sub[(size_t) i].stopNote();
            currentNote[(size_t) i] = -1;
        }
    }
}

void LardossaVoicePool::panic()
{
    for (int i = 0; i < kNumVoices; ++i)
    {
        fm[(size_t) i].noteOff();
        sub[(size_t) i].stopNote();
        currentNote[(size_t) i] = -1;
    }
}

void LardossaVoicePool::setGlideAll(float glideMs, bool fmEngine)
{
    if (fmEngine)
    {
        for (auto& v : fm)
            v.setGlideMs(glideMs);
    }
    else
    {
        // glide applied in updateSubVoice per voice
        juce::ignoreUnused(glideMs);
    }
}

void LardossaVoicePool::updateFmVoice(int idx, int algorithm, const float* opRatio, const float* opDetune,
                                      const float* opLevel, float op1Fb,
                                      const float* opAtk, const float* opDec, const float* opSus, const float* opRel)
{
    if (!juce::isPositiveAndBelow(idx, kNumVoices) || opRatio == nullptr)
        return;
    auto& v = fm[(size_t) idx];
    v.setAlgorithm(algorithm);
    for (int o = 0; o < FmVoice::kNumOps; ++o)
    {
        const float fb = (o == 0 ? op1Fb : 0.f);
        v.setOperatorParams(o, opRatio[(size_t) o], opDetune[(size_t) o], opLevel[(size_t) o], fb);
        v.setOperatorEnv(o, opAtk[(size_t) o], opDec[(size_t) o], opSus[(size_t) o], opRel[(size_t) o]);
    }
}

void LardossaVoicePool::updateSubVoice(int idx, float oscDetuneCents, float voltage, float depth, float pressure, float decay,
                                       float ampA, float ampD, float ampS, float ampR, float filterDrive, float glideMs)
{
    if (!juce::isPositiveAndBelow(idx, kNumVoices))
        return;
    sub[(size_t) idx].updateParams(0, 0.f, 0.5f, oscDetuneCents, 0.f, 0.f, filterDrive, glideMs,
                                   voltage, depth, pressure, decay, ampA, ampD, ampS, ampR);
}

void LardossaVoicePool::renderFmVoiceAdding(int voiceIdx, float* mono, int destStart, int numSamples)
{
    if (!juce::isPositiveAndBelow(voiceIdx, kNumVoices) || mono == nullptr || numSamples <= 0)
        return;
    auto& v = fm[(size_t) voiceIdx];
    if (!v.isActive())
        return;
    for (int i = 0; i < numSamples; ++i)
        mono[destStart + i] += v.process();
}

void LardossaVoicePool::renderSubVoiceAdding(int voiceIdx, juce::AudioBuffer<float>& buffer, int destStart, int numSamples)
{
    if (!juce::isPositiveAndBelow(voiceIdx, kNumVoices) || numSamples <= 0)
        return;
    auto& v = sub[(size_t) voiceIdx];
    if (!v.isActive())
        return;
    v.renderAddingTo(buffer, destStart, numSamples);
}

void LardossaVoicePool::startSeqNoteFm(int midi, float vel, bool accent, bool legato, bool slide)
{
    juce::ignoreUnused(legato, slide);
    auto& v = fm[0];
    const float vv = juce::jlimit(0.01f, 1.f, vel + (accent ? 0.12f : 0.f));
    v.noteOff();
    v.noteOn(midi, vv);
    currentNote[0] = midi;
}

void LardossaVoicePool::startSeqNoteSub(int midi, float vel, bool accent, bool legato, bool slide)
{
    auto& v = sub[0];
    v.startNote(midi, vel, accent, legato, slide);
    currentNote[0] = midi;
}

void LardossaVoicePool::stopSeqVoiceFm()
{
    fm[0].noteOff();
    currentNote[0] = -1;
}

void LardossaVoicePool::stopSeqVoiceSub()
{
    sub[0].stopNote();
    currentNote[0] = -1;
}
