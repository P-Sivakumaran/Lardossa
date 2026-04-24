#include "PluginProcessor.h"
#include "Parameters.h"
#include "PresetManager.h"
#include "PluginEditor.h"
#include "PatternGenerators.h"
#include "SequencerTiming.h"
#include "TransportResolver.h"
#include "WebStepPatch.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::Identifier kStateVersionKey{"stateVersion"};

juce::String stepPrefix(int index)
{
    return "seqStep" + juce::String(index) + "_";
}

struct SeqStepEvent
{
    int sample = 0;
    int prevIdx = 0;
    int newStep = 0;
};
} // namespace

LardossaAudioProcessor::LardossaAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, &apvtsUndoManager, "PARAMS", createLardossaParameterLayout())
{
    p_engineMode = apvts.getRawParameterValue("engineMode");
    p_algorithm = apvts.getRawParameterValue("algorithm");
    p_op1Ratio = apvts.getRawParameterValue("op1Ratio");
    p_op2Ratio = apvts.getRawParameterValue("op2Ratio");
    p_op3Ratio = apvts.getRawParameterValue("op3Ratio");
    p_op4Ratio = apvts.getRawParameterValue("op4Ratio");
    p_op1Level = apvts.getRawParameterValue("op1Level");
    p_op2Level = apvts.getRawParameterValue("op2Level");
    p_op3Level = apvts.getRawParameterValue("op3Level");
    p_op4Level = apvts.getRawParameterValue("op4Level");
    p_op1Feedback = apvts.getRawParameterValue("op1Feedback");
    p_voltage = apvts.getRawParameterValue("voltage");
    p_depth = apvts.getRawParameterValue("depth");
    p_pressure = apvts.getRawParameterValue("pressure");
    p_decay = apvts.getRawParameterValue("decay");
    p_ampAttackMs = apvts.getRawParameterValue("ampAttackMs");
    p_ampDecayMs = apvts.getRawParameterValue("ampDecayMs");
    p_ampSustain = apvts.getRawParameterValue("ampSustain");
    p_ampReleaseMs = apvts.getRawParameterValue("ampReleaseMs");
    p_glideMs = apvts.getRawParameterValue("glideMs");
    p_oscDetune = apvts.getRawParameterValue("oscDetune");
    p_seqLength = apvts.getRawParameterValue("seqLength");
    p_seqRecord = apvts.getRawParameterValue("seqRecord");
    p_standalonePlay = apvts.getRawParameterValue("standalonePlay");
    p_standaloneBpm = apvts.getRawParameterValue("standaloneBpm");
    p_seqVariety = apvts.getRawParameterValue("seqVariety");
    p_outputGain = apvts.getRawParameterValue("outputGain");
    p_filterDrive = apvts.getRawParameterValue("filterDrive");

    for (int i = 0; i < 16; i += 4)
        sequencerSteps[(size_t) i].enabled = true;

    patterns::generateDetroitBass(sequencerSteps, rng, 16, 0.5f);
    sequencerStepsAudioCache = sequencerSteps;
}

LardossaAudioProcessor::~LardossaAudioProcessor() = default;

const juce::String LardossaAudioProcessor::getName() const { return JucePlugin_Name; }
bool LardossaAudioProcessor::acceptsMidi() const { return true; }
bool LardossaAudioProcessor::producesMidi() const { return false; }
bool LardossaAudioProcessor::isMidiEffect() const { return false; }
double LardossaAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int LardossaAudioProcessor::getNumPrograms() { return 1; }
int LardossaAudioProcessor::getCurrentProgram() { return 0; }
void LardossaAudioProcessor::setCurrentProgram(int) {}
const juce::String LardossaAudioProcessor::getProgramName(int) { return {}; }
void LardossaAudioProcessor::changeProgramName(int, const juce::String&) {}

void LardossaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    panicRequested.store(false);
    voicePool.prepare(sampleRate, samplesPerBlock);
    transportState_ = {};
    seqGateEndPpq.reset();
    lastSequencerNote = -1;
    lastStepIndex.store(-1);
    voicePool.panic();
}

void LardossaAudioProcessor::releaseResources() {}

bool LardossaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

int LardossaAudioProcessor::getSequencerLength() const
{
    if (auto* pi = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("seqLength")))
        return pi->get();
    return 16;
}

LardossaAudioProcessor::TransportContext LardossaAudioProcessor::resolveTransportContext(int numSamples)
{
    return lardossa::transport::resolveBlock(transportState_,
                                             wrapperType,
                                             getPlayHead(),
                                             getSampleRate() > 0.0 ? getSampleRate() : 44100.0,
                                             numSamples,
                                             p_standalonePlay != nullptr && p_standalonePlay->load() > 0.5f,
                                             p_standaloneBpm != nullptr ? (double) p_standaloneBpm->load() : 120.0);
}

void LardossaAudioProcessor::updateVoiceParamsFromApvts()
{
    const bool fmEngine = p_engineMode == nullptr || p_engineMode->load() > 0.5f;
    int algo = 1;
    if (auto* pi = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("algorithm")))
        algo = juce::jlimit(1, 5, pi->get());
    float ratios[4] = {1, 2, 3, 7};
    float det[4] = {0, 0, 0, 0};
    float lvls[4] = {1, 0.8f, 0.5f, 0.3f};
    float atk[4], dec[4], sus[4], rel[4];
    if (p_op1Ratio != nullptr)
        ratios[0] = p_op1Ratio->load();
    if (p_op2Ratio != nullptr)
        ratios[1] = p_op2Ratio->load();
    if (p_op3Ratio != nullptr)
        ratios[2] = p_op3Ratio->load();
    if (p_op4Ratio != nullptr)
        ratios[3] = p_op4Ratio->load();
    if (p_op1Level != nullptr)
        lvls[0] = p_op1Level->load();
    if (p_op2Level != nullptr)
        lvls[1] = p_op2Level->load();
    if (p_op3Level != nullptr)
        lvls[2] = p_op3Level->load();
    if (p_op4Level != nullptr)
        lvls[3] = p_op4Level->load();
    const float aA = p_ampAttackMs != nullptr ? p_ampAttackMs->load() : 2.f;
    const float aD = p_ampDecayMs != nullptr ? p_ampDecayMs->load() : 100.f;
    const float aS = p_ampSustain != nullptr ? p_ampSustain->load() : 0.7f;
    const float aR = p_ampReleaseMs != nullptr ? p_ampReleaseMs->load() : 200.f;
    for (int o = 0; o < 4; ++o)
    {
        atk[(size_t) o] = aA;
        dec[(size_t) o] = aD;
        sus[(size_t) o] = aS;
        rel[(size_t) o] = aR;
    }
    const float fb = p_op1Feedback != nullptr ? p_op1Feedback->load() : 0.f;
    const float glide = p_glideMs != nullptr ? p_glideMs->load() : 0.f;
    const float volt = p_voltage != nullptr ? p_voltage->load() : 800.f;
    const float depth = p_depth != nullptr ? p_depth->load() : 0.5f;
    const float pressure = p_pressure != nullptr ? p_pressure->load() : 0.5f;
    const float decay = p_decay != nullptr ? p_decay->load() : 200.f;
    const float fDrive = p_filterDrive != nullptr ? p_filterDrive->load() : 0.2f;
    const float oscDet = p_oscDetune != nullptr ? p_oscDetune->load() : 0.f;

    voicePool.setGlideAll(glide, fmEngine);
    for (int i = 0; i < LardossaVoicePool::kNumVoices; ++i)
    {
        voicePool.updateFmVoice(i, algo - 1, ratios, det, lvls, fb, atk, dec, sus, rel);
        voicePool.updateSubVoice(i, oscDet, volt, depth, pressure, decay, aA, aD, aS, aR, fDrive, glide);
    }
}

const SequencerStepArray& LardossaAudioProcessor::snapshotSequencerStepsForAudioThread()
{
    const juce::SpinLock::ScopedTryLockType tryLock(sequencerLock);
    if (tryLock.isLocked())
        sequencerStepsAudioCache = sequencerSteps;
    return sequencerStepsAudioCache;
}

std::optional<double> LardossaAudioProcessor::applySequencerStepChange(int prevIdx, int currentStep, double stepOnsetPpq,
                                                                      const TransportContext& transport,
                                                                      const SequencerStepArray& stepsCopy, bool fmEngine)
{
    const int seqLength = getSequencerLength();
    const auto& prev = stepsCopy[(size_t) prevIdx];
    const auto& cur = stepsCopy[(size_t) currentStep];

    const bool wrappedAcrossPattern = (prevIdx == seqLength - 1 && currentStep == 0);
    const bool hadPitch = (lastSequencerNote >= 0);
    bool carryOver = prev.enabled && cur.enabled && (prev.tie || prev.slide || cur.slide);
    if (wrappedAcrossPattern && prev.tie && cur.enabled && (!cur.tie) && hadPitch)
        carryOver = false;

    const bool glideLegato = prev.enabled && cur.enabled && (prev.slide || cur.slide);

    if (hadPitch && !carryOver)
    {
        if (fmEngine)
            voicePool.stopSeqVoiceFm();
        else
            voicePool.stopSeqVoiceSub();
        lastSequencerNote = -1;
    }

    if (cur.enabled)
    {
        const auto accentBoost = cur.accent ? 0.62f : 0.0f;
        const auto vel = juce::jlimit(0.01f, 1.0f, cur.velocity + accentBoost);
        if (carryOver && lastSequencerNote >= 0)
        {
            if (cur.note != lastSequencerNote)
            {
                if (glideLegato)
                {
                    if (fmEngine)
                        voicePool.startSeqNoteFm(cur.note, vel, cur.accent, true, cur.slide);
                    else
                        voicePool.startSeqNoteSub(cur.note, vel, cur.accent, true, cur.slide);
                }
                else
                {
                    if (fmEngine)
                    {
                        voicePool.stopSeqVoiceFm();
                        voicePool.startSeqNoteFm(cur.note, vel, cur.accent, false, cur.slide);
                    }
                    else
                    {
                        voicePool.stopSeqVoiceSub();
                        voicePool.startSeqNoteSub(cur.note, vel, cur.accent, false, cur.slide);
                    }
                }
            }
        }
        else
        {
            if (fmEngine)
                voicePool.startSeqNoteFm(cur.note, vel, cur.accent, false, cur.slide);
            else
                voicePool.startSeqNoteSub(cur.note, vel, cur.accent, false, cur.slide);
        }

        lastSequencerNote = cur.note;

        if (!cur.tie)
        {
            const auto secPer16th = (60.0 / transport.bpmForBlock) * 0.25;
            const float L = juce::jmax(0.25f, seq::normaliseStepLength(cur.lengthFrac));
            const int stepSamples = juce::jmax(1, (int) std::round(secPer16th * transport.sampleRate * (double) L));
            const float gateMul = cur.slide ? 0.96f : 0.45f;
            const int gateSamples = juce::jmax(1, (int) std::round((float) stepSamples * gateMul));
            const double gateDurPpq = ((double) gateSamples / transport.sampleRate) * (transport.bpmForBlock / 60.0);
            return stepOnsetPpq + gateDurPpq;
        }
    }

    return std::nullopt;
}

void LardossaAudioProcessor::processSequencerVoice(juce::AudioBuffer<float>& buffer, float* monoFmScratch,
                                                   const TransportContext& transport, const SequencerStepArray& stepsCopy,
                                                   bool fmEngine)
{
    if (!transport.isPlaying)
    {
        if (lastSequencerNote >= 0)
        {
            if (fmEngine)
                voicePool.stopSeqVoiceFm();
            else
                voicePool.stopSeqVoiceSub();
        }
        lastSequencerNote = -1;
        lastStepIndex.store(-1);
        seqGateEndPpq.reset();
        return;
    }

    const int seqLength = getSequencerLength();
    const int n = transport.numSamples;
    if (n <= 0 || transport.sampleRate <= 0.0)
        return;

    const double ppqPerSample = (transport.bpmForBlock / 60.0) / transport.sampleRate;
    const double ppq0 = transport.ppqPosition;
    const double ppqEnd = ppq0 + ppqPerSample * (double) n;

    auto stepAt = [&](double ppq)
    {
        return seq::stepIndexFromPatternPpq(ppq, seqLength, stepsCopy);
    };

    auto ppqAtSample = [&](int sampleInBlock)
    {
        return ppq0 + ppqPerSample * (double) sampleInBlock;
    };

    auto sampleAtOrAfterPpq = [&](double targetPpq) -> int
    {
        if (targetPpq <= ppq0)
            return 0;
        int lo = 0, hi = n;
        while (lo < hi)
        {
            const int mid = (lo + hi) / 2;
            const double p = ppqAtSample(mid);
            if (p < targetPpq)
                lo = mid + 1;
            else
                hi = mid;
        }
        return juce::jlimit(0, n, lo);
    };

    int scanLast = lastStepIndex.load();
    const int targetAtEnd = stepAt(ppqEnd - 1.0e-9);

    SeqStepEvent events[64];
    int numEvents = 0;

    const int stepAtStart = stepAt(ppq0 + 1.0e-9);
    if (stepAtStart != scanLast)
    {
        const int pIdx = (stepAtStart - 1 + seqLength) % seqLength;
        if (numEvents < 64)
            events[numEvents++] = {0, pIdx, stepAtStart};
        scanLast = stepAtStart;
    }

    while (scanLast != targetAtEnd)
    {
        int lo = 1, hi = n;
        while (lo < hi)
        {
            const int mid = (lo + hi) / 2;
            const int sMid = stepAt(ppq0 + ppqPerSample * (double) mid);
            if (sMid == scanLast)
                lo = mid + 1;
            else
                hi = mid;
        }

        const int newStep = stepAt(ppq0 + ppqPerSample * (double) lo);
        const int prevIdx = (newStep - 1 + seqLength) % seqLength;
        if (numEvents < 64)
            events[numEvents++] = {lo, prevIdx, newStep};
        scanLast = newStep;
    }

    std::optional<double> pendingGateEnd;

    if (seqGateEndPpq)
    {
        const double g = *seqGateEndPpq;
        if (g <= ppq0)
        {
            if (fmEngine)
                voicePool.stopSeqVoiceFm();
            else
                voicePool.stopSeqVoiceSub();
            lastSequencerNote = -1;
            seqGateEndPpq.reset();
        }
        else if (g <= ppqEnd)
        {
            pendingGateEnd = seqGateEndPpq;
            seqGateEndPpq.reset();
        }
    }

    auto flushGatesBefore = [&](int targetSample, int& cursor, bool& didRender)
    {
        while (pendingGateEnd && *pendingGateEnd <= ppqAtSample(targetSample) + 1.0e-9)
        {
            int offS = sampleAtOrAfterPpq(*pendingGateEnd);
            offS = juce::jlimit(cursor, targetSample, offS);
            if (offS > cursor)
            {
                if (fmEngine && monoFmScratch != nullptr)
                {
                    voicePool.renderFmVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, monoFmScratch, cursor, offS - cursor);
                    didRender = true;
                }
                else
                {
                    voicePool.renderSubVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, buffer, cursor, offS - cursor);
                    didRender = true;
                }
            }
            cursor = offS;
            if (fmEngine)
                voicePool.stopSeqVoiceFm();
            else
                voicePool.stopSeqVoiceSub();
            lastSequencerNote = -1;
            pendingGateEnd.reset();
        }
    };

    int cursor = 0;
    bool didSeqRender = false;

    for (int ei = 0; ei < numEvents; ++ei)
    {
        const auto& ev = events[ei];
        flushGatesBefore(ev.sample, cursor, didSeqRender);

        if (ev.sample > cursor)
        {
            if (fmEngine && monoFmScratch != nullptr)
            {
                voicePool.renderFmVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, monoFmScratch, cursor, ev.sample - cursor);
                didSeqRender = true;
            }
            else
            {
                voicePool.renderSubVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, buffer, cursor, ev.sample - cursor);
                didSeqRender = true;
            }
        }

        pendingGateEnd = applySequencerStepChange(ev.prevIdx, ev.newStep, ppqAtSample(ev.sample), transport, stepsCopy, fmEngine);
        cursor = ev.sample;
    }

    flushGatesBefore(n, cursor, didSeqRender);

    if (cursor < n)
    {
        if (fmEngine && monoFmScratch != nullptr)
        {
            voicePool.renderFmVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, monoFmScratch, cursor, n - cursor);
            didSeqRender = true;
        }
        else
        {
            voicePool.renderSubVoiceAdding(LardossaVoicePool::kSeqVoiceSlot, buffer, cursor, n - cursor);
            didSeqRender = true;
        }
    }

    if (didSeqRender)
        seqVoiceRenderedThisBlock = true;

    lastStepIndex.store(targetAtEnd);

    if (pendingGateEnd && *pendingGateEnd > ppqEnd)
        seqGateEndPpq = pendingGateEnd;
}

void LardossaAudioProcessor::processExternalMidi(const juce::MidiBuffer& midiMessages, bool seqRecordArmed, bool fmEngine)
{
    const auto liveStep = lastStepIndex.load();

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            if (seqRecordArmed && juce::isPositiveAndBelow(liveStep, getSequencerLength()))
            {
                const juce::SpinLock::ScopedTryLockType lock(sequencerLock);
                if (lock.isLocked())
                {
                    auto& step = sequencerSteps[(size_t) liveStep];
                    step.enabled = true;
                    step.note = msg.getNoteNumber();
                    step.velocity = juce::jlimit(0.05f, 1.0f, msg.getFloatVelocity());
                }
            }

            for (int i = 1; i < LardossaVoicePool::kNumVoices; ++i)
                if (voicePool.getSlotMidi(i) == msg.getNoteNumber())
                {
                    if (fmEngine)
                        voicePool.noteOff(msg.getNoteNumber(), true);
                    else
                        voicePool.noteOff(msg.getNoteNumber(), false);
                }

            voicePool.noteOn(msg.getNoteNumber(), msg.getFloatVelocity(), fmEngine);
        }
        else if (msg.isNoteOff())
        {
            if (fmEngine)
                voicePool.noteOff(msg.getNoteNumber(), true);
            else
                voicePool.noteOff(msg.getNoteNumber(), false);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            voicePool.panic();
    }
}

void LardossaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    seqVoiceRenderedThisBlock = false;

    if (panicRequested.exchange(false))
    {
        voicePool.panic();
        lastSequencerNote = -1;
        seqGateEndPpq.reset();
        for (int c = 1; c <= 16; ++c)
            keyboardState.allNotesOff(c);
    }

    juce::MidiBuffer midiCopy;
    midiCopy.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    const auto transport = resolveTransportContext(buffer.getNumSamples());
    lastKnownBpm = transport.bpmForBlock;

    const bool fmEngine = p_engineMode == nullptr || p_engineMode->load() > 0.5f;
    updateVoiceParamsFromApvts();

    const int n = buffer.getNumSamples();
    float monoFm[8192];
    jassert(n <= 8192);
    if (n > 8192)
        return;
    if (fmEngine)
        juce::FloatVectorOperations::clear(monoFm, n);

    if (transport.isPlaying)
    {
        const SequencerStepArray& stepsCopy = snapshotSequencerStepsForAudioThread();
        processSequencerVoice(buffer, monoFm, transport, stepsCopy, fmEngine);
    }
    else
        processSequencerVoice(buffer, monoFm, transport, sequencerStepsAudioCache, fmEngine);

    processExternalMidi(midiCopy, p_seqRecord != nullptr && p_seqRecord->load() > 0.5f, fmEngine);

    if (fmEngine)
    {
        for (int v = 1; v < LardossaVoicePool::kNumVoices; ++v)
        {
            if (seqVoiceRenderedThisBlock && v == LardossaVoicePool::kSeqVoiceSlot)
                continue;
            voicePool.renderFmVoiceAdding(v, monoFm, 0, n);
        }

        float* L = buffer.getWritePointer(0);
        float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : L;
        const float drive = juce::jmax(0.01f, p_filterDrive != nullptr ? p_filterDrive->load() : 0.2f);
        const float invT = 1.f / std::tanh(drive);
        for (int i = 0; i < n; ++i)
        {
            float s = std::tanh(monoFm[i] * drive) * invT;
            L[i] = s;
            R[i] = s;
        }
    }
    else
    {
        for (int v = 1; v < LardossaVoicePool::kNumVoices; ++v)
        {
            if (seqVoiceRenderedThisBlock && v == LardossaVoicePool::kSeqVoiceSlot)
                continue;
            voicePool.renderSubVoiceAdding(v, buffer, 0, n);
        }

        float* L = buffer.getWritePointer(0);
        float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : L;
        const float drive = juce::jmax(0.01f, p_filterDrive != nullptr ? p_filterDrive->load() : 0.2f);
        const float invT = 1.f / std::tanh(drive);
        for (int i = 0; i < n; ++i)
        {
            float s = std::tanh(L[i] * drive) * invT;
            L[i] = s;
            R[i] = s;
        }
    }

    const auto gainDb = p_outputGain != nullptr ? p_outputGain->load() : 0.f;
    buffer.applyGain(juce::Decibels::decibelsToGain(gainDb));
}

bool LardossaAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* LardossaAudioProcessor::createEditor() { return new LardossaAudioProcessorEditor(*this); }

void LardossaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (!state.isValid())
        return;
    state.setProperty(kStateVersionKey, LardossaAudioProcessor::kStateVersion, nullptr);
    saveSequencerStepsToState(state);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void LardossaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto state = juce::ValueTree::fromXml(*xml);
        if (!state.isValid())
            return;
        const int fileVersion = (int) state.getProperty(kStateVersionKey, (int) LardossaAudioProcessor::kStateVersion);
        if (fileVersion > LardossaAudioProcessor::kStateVersion)
            return;
        apvts.replaceState(state);
        restoreSequencerStepsFromState(state);
    }
}

void LardossaAudioProcessor::saveSequencerStepsToState(juce::ValueTree& state) const
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    for (int i = 0; i < kNumSequencerSteps; ++i)
    {
        const auto& s = sequencerSteps[(size_t) i];
        const auto prefix = stepPrefix(i);
        state.setProperty(prefix + "en", s.enabled, nullptr);
        state.setProperty(prefix + "note", s.note, nullptr);
        state.setProperty(prefix + "vel", (double) s.velocity, nullptr);
        state.setProperty(prefix + "acc", s.accent, nullptr);
        state.setProperty(prefix + "sld", s.slide, nullptr);
        state.setProperty(prefix + "tie", s.tie, nullptr);
        state.setProperty(prefix + "len", (double) seq::normaliseStepLength(s.lengthFrac), nullptr);
    }
}

void LardossaAudioProcessor::restoreSequencerStepsFromState(const juce::ValueTree& state)
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    for (int i = 0; i < kNumSequencerSteps; ++i)
    {
        auto& s = sequencerSteps[(size_t) i];
        const auto prefix = stepPrefix(i);
        s.enabled = (bool) state.getProperty(prefix + "en", false);
        s.note = juce::jlimit(0, 127, (int) state.getProperty(prefix + "note", 57));
        s.velocity = juce::jlimit(0.01f, 1.0f, (float) (double) state.getProperty(prefix + "vel", 0.85));
        s.accent = (bool) state.getProperty(prefix + "acc", false);
        s.slide = (bool) state.getProperty(prefix + "sld", false);
        s.tie = (bool) state.getProperty(prefix + "tie", false);
        s.lengthFrac = seq::normaliseStepLength((float) (double) state.getProperty(prefix + "len", 1.0));
    }
}

void LardossaAudioProcessor::replaceSequencerStepFromWeb(int stepIndex, int note, float velocity, bool accent, bool slide,
                                                         bool tie, float lengthInSixteenths, bool enabled)
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    if (!juce::isPositiveAndBelow(stepIndex, kNumSequencerSteps))
        return;
    auto& s = sequencerSteps[(size_t) stepIndex];
    s.note = juce::jlimit(0, 127, note);
    s.velocity = juce::jlimit(0.01f, 1.0f, velocity);
    s.accent = accent;
    s.slide = slide;
    s.tie = tie;
    s.lengthFrac = seq::normaliseStepLength(lengthInSixteenths);
    s.enabled = enabled;
}

void LardossaAudioProcessor::setSequencerAccentFromWeb(int stepIndex, bool accent)
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    if (juce::isPositiveAndBelow(stepIndex, kNumSequencerSteps))
        sequencerSteps[(size_t) stepIndex].accent = accent;
}

void LardossaAudioProcessor::setSequencerSlideFromWeb(int stepIndex, bool slide)
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    if (juce::isPositiveAndBelow(stepIndex, kNumSequencerSteps))
        sequencerSteps[(size_t) stepIndex].slide = slide;
}

void LardossaAudioProcessor::setSequencerTieFromWeb(int stepIndex, bool tie)
{
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    if (juce::isPositiveAndBelow(stepIndex, kNumSequencerSteps))
        sequencerSteps[(size_t) stepIndex].tie = tie;
}

void LardossaAudioProcessor::applyFactoryPresetByName(const juce::String& name)
{
    PresetManager::applyFactoryPreset(apvts, name.trim());
}

juce::StringArray LardossaAudioProcessor::getFactoryPresetNames() const
{
    return PresetManager::getFactoryPresetNames();
}

void LardossaAudioProcessor::handleWebNativeEvent(const juce::String& eventType, const juce::var& payload)
{
    if (eventType == "paramChange")
    {
        const auto paramId = payload["id"].toString();
        const auto value = (float) payload["value"];
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost((float) param->convertTo0to1((double) value));
    }
    else if (eventType == "panicAll")
    {
        juce::MessageManager::callAsync([this]
                                        { panicRequested.store(true); });
    }
    else if (eventType == "seqStep" || eventType == "setStep")
        lardossa::sequencer::applyWebStep(*this, payload);
    else if (eventType == "setAccentStep")
        lardossa::sequencer::applyWebStepAccent(*this, payload);
    else if (eventType == "setSlideStep")
        lardossa::sequencer::applyWebStepSlide(*this, payload);
    else if (eventType == "setTieStep")
        lardossa::sequencer::applyWebStepTie(*this, payload);
    else if (eventType == "generatePattern")
    {
        const auto mode = payload["mode"].toString();
        const float variety = (float) payload.getProperty("variety", 0.5f);
        const juce::SpinLock::ScopedLockType lock(sequencerLock);
        const int len = getSequencerLength();
        if (mode == "DetroitBass")
            patterns::generateDetroitBass(sequencerSteps, rng, len, variety);
        else if (mode == "DetroitLead")
            patterns::generateDetroitLead(sequencerSteps, rng, len, variety);
        else if (mode == "Electro")
            patterns::generateElectroGroove(sequencerSteps, rng, len, variety);
    }
    else if (eventType == "loadFactoryPreset")
    {
        applyFactoryPresetByName(payload["name"].toString());
    }
}

juce::String LardossaAudioProcessor::getUiStateJson()
{
    auto* obj = new juce::DynamicObject();

    auto addParamHz = [&](const char* id)
    {
        if (auto* raw = apvts.getRawParameterValue(id))
            obj->setProperty(id, raw->load());
    };

    addParamHz("engineMode");
    if (auto* pi = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("algorithm")))
        obj->setProperty("algorithm", pi->get());
    else
        obj->setProperty("algorithm", 1);

    addParamHz("op1Ratio");
    addParamHz("op2Ratio");
    addParamHz("op3Ratio");
    addParamHz("op4Ratio");
    addParamHz("op1Level");
    addParamHz("op2Level");
    addParamHz("op3Level");
    addParamHz("op4Level");
    addParamHz("op1Feedback");
    addParamHz("voltage");
    addParamHz("depth");
    addParamHz("pressure");
    addParamHz("decay");
    addParamHz("ampAttackMs");
    addParamHz("ampDecayMs");
    addParamHz("ampSustain");
    addParamHz("ampReleaseMs");
    addParamHz("glideMs");
    addParamHz("oscDetune");

    if (auto* pi = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter("seqLength")))
        obj->setProperty("seqLength", pi->get());
    else
        obj->setProperty("seqLength", 16);

    addParamHz("seqRecord");
    addParamHz("standalonePlay");
    addParamHz("standaloneBpm");
    addParamHz("seqVariety");
    addParamHz("outputGain");
    addParamHz("filterDrive");

    juce::Array<juce::var> presetArr;
    for (auto& n : getFactoryPresetNames())
        presetArr.add(n);
    obj->setProperty("presets", juce::var(presetArr));

    juce::Array<juce::var> stepArr;
    const int seqLen = juce::jlimit(1, kNumSequencerSteps, getSequencerLength());
    const juce::SpinLock::ScopedLockType lock(sequencerLock);
    for (int i = 0; i < seqLen; ++i)
    {
        const auto& s = sequencerSteps[(size_t) i];
        auto* st = new juce::DynamicObject();
        st->setProperty("index", i);
        st->setProperty("on", s.enabled);
        st->setProperty("note", s.note);
        st->setProperty("vel", (double) s.velocity);
        st->setProperty("accent", s.accent);
        st->setProperty("slide", s.slide);
        st->setProperty("tie", s.tie);
        st->setProperty("len", (double) s.lengthFrac);
        stepArr.add(juce::var(st));
    }
    obj->setProperty("seqSteps", juce::var(stepArr));

    return juce::JSON::toString(juce::var(obj));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LardossaAudioProcessor();
}
