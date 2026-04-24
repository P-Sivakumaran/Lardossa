#pragma once

/** Full UI state shape (C++ mirror for documentation / future binary bridge). */
struct LardossaState
{
    int engineMode = 1;
    int algorithm = 1;
    float op1Ratio = 1.f;
    float op2Ratio = 2.f;
    float op3Ratio = 3.f;
    float op4Ratio = 7.f;
    float op1Level = 1.f;
    float op2Level = 0.8f;
    float op3Level = 0.5f;
    float op4Level = 0.3f;
    float op1Feedback = 0.f;
    float voltage = 800.f;
    float depth = 0.5f;
    float pressure = 0.5f;
    float decay = 200.f;
    float ampAttackMs = 2.f;
    float ampDecayMs = 100.f;
    float ampSustain = 0.7f;
    float ampReleaseMs = 200.f;
    float glideMs = 0.f;
    float oscDetune = 0.f;
    int seqLength = 16;
    bool seqRecord = false;
    bool standalonePlay = false;
    float standaloneBpm = 120.f;
    float seqVariety = 0.5f;
    float outputGain = 0.f;
    float filterDrive = 0.2f;
};
