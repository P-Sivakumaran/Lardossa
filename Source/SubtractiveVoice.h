#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/** Subtractive voice (JUCE ladder filter + ADSR) — ported from Mordtakt for `engineMode` Sub. */
class SubtractiveVoice final
{
public:
    void startNote(int midiNoteNumber, float velocity, bool accentStep = false, bool legatoNoRetrigger = false, bool slideStep = false);
    void stopNote();

    bool isActive() const { return noteActive; }

    void setSampleRate(double newSampleRate, int maxBlockSize = 512);

    void updateParams(int oscShape,
                       float subLevel_,
                       float pulseWidth_,
                       float osc2Detune_,
                       float osc2Mix_,
                       float noiseLevel_,
                       float filterDrive_,
                       float glideMs_,
                       float cutoffHz,
                       float resonance,
                       float filterEnvAmount,
                       float filterDecayMs,
                       float ampAttackMs,
                       float ampDecayMs,
                       float ampSustain,
                       float ampReleaseMs);

    void render(juce::AudioBuffer<float>& outputBuffer, int numSamples);
    void renderAddingTo(juce::AudioBuffer<float>& outputBuffer, int destStartSample, int numSamples);

private:
    float renderOsc(float phase, float dt, int shape);

    juce::dsp::LadderFilter<float> filter;
    juce::ADSR ampEnvelope, filterEnvelope;
    juce::AudioBuffer<float> tempBuffer;

    float sr = 44100.0f;
    float phase1 = 0.0f, phase2 = 0.0f, subPhase = 0.0f;
    float currentFreq = 0.0f, targetFreq = 440.0f;
    float glideCoef = 0.0f;
    float lastGlideMs = 0.0f;
    bool glideActive = false;

    int waveShape = 0;
    float level = 0.8f;
    float noteVelocity = 0.85f;
    float baseCutoff = 700.0f;
    float envAmount = 0.6f;
    float subLevel = 0.0f;
    float pulseWidth = 0.5f;
    float osc2Detune = 0.0f;
    float osc2FreqRatio = 1.0f;
    float osc2Mix = 0.0f;
    float noiseLevel = 0.0f;
    float filterDrive = 0.0f;
    float driveGain = 1.0f;
    float invTanhDriveGain = 1.0f;
    float voiceResonance = 0.45f;
    bool accentActive = false;
    float triState = 0.0f;
    uint32_t noiseState = 0xDEADBEEF;
    bool noteActive = false;
};
