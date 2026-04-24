#include <catch2/catch_test_macros.hpp>

#include <FmVoice.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

static float rms(const std::vector<float>& x)
{
    double s = 0;
    for (float v : x)
        s += (double) v * (double) v;
    const int n = (int) x.size();
    return n > 0 ? (float) std::sqrt(s / (double) n) : 0.f;
}

static void defaultFmPatch(FmVoice& v)
{
    v.setSampleRate(44100.f);
    v.setAlgorithm(0);
    const float ratios[4] = {1.f, 2.f, 3.f, 7.f};
    const float lv[4] = {1.f, 0.8f, 0.5f, 0.3f};
    for (int o = 0; o < FmVoice::kNumOps; ++o)
    {
        v.setOperatorParams(o, ratios[(size_t) o], 0.f, lv[(size_t) o], 0.f);
        v.setOperatorEnv(o, 2.f, 100.f, 0.8f, 200.f);
    }
}

TEST_CASE("nonZeroAudioAfterNoteOn", "[fm]")
{
    FmVoice v;
    defaultFmPatch(v);
    v.noteOn(60, 1.f);
    std::vector<float> buf(64);
    for (int i = 0; i < 64; ++i)
        buf[(size_t) i] = v.process();
    REQUIRE(rms(buf) > 0.001f);
}

TEST_CASE("allAlgorithmsDistinct", "[fm]")
{
    std::array<float, 5> levels{};
    for (int a = 0; a < 5; ++a)
    {
        FmVoice v;
        defaultFmPatch(v);
        v.setAlgorithm(a);
        v.noteOn(60, 1.f);
        std::vector<float> buf(512);
        for (int i = 0; i < 512; ++i)
            buf[(size_t) i] = v.process();
        levels[(size_t) a] = rms(buf);
    }
    const float lo = *std::min_element(levels.begin(), levels.end());
    const float hi = *std::max_element(levels.begin(), levels.end());
    REQUIRE((hi - lo) > 0.02f * (std::max)(1.f, hi));
}

TEST_CASE("noteOffDecaysToSilence", "[fm]")
{
    FmVoice v;
    defaultFmPatch(v);
    v.setOperatorEnv(0, 2.f, 100.f, 0.8f, 50.f);
    v.setOperatorEnv(1, 2.f, 100.f, 0.8f, 50.f);
    v.setOperatorEnv(2, 2.f, 100.f, 0.8f, 50.f);
    v.setOperatorEnv(3, 2.f, 100.f, 0.8f, 50.f);
    v.noteOn(60, 1.f);
    v.noteOff();
    const int relSamples = (int) std::ceil(2.f * 50.f * 0.001f * 44100.f) + 500;
    float peak = 0.f;
    for (int i = 0; i < relSamples; ++i)
        peak = std::max(peak, std::abs(v.process()));
    REQUIRE(peak < 0.0001f);
}

TEST_CASE("glideConverges", "[fm]")
{
    FmVoice v;
    defaultFmPatch(v);
    v.setGlideMs(50.f);
    v.noteOn(60, 1.f);
    for (int i = 0; i < 2000; ++i)
        v.process();
    v.noteOn(72, 1.f);
    const int extra = (int) std::ceil(60.f * 0.001f * 44100.f);
    for (int i = 0; i < extra; ++i)
        v.process();
    const float out = v.process();
    REQUIRE(std::abs(out) < 1.0f);
}
