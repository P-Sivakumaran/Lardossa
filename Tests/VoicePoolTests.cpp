#include <catch2/catch_test_macros.hpp>

#include <VoicePool.h>
#include <cmath>
#include <vector>

TEST_CASE("voice pool FM render is finite after many note-ons", "[voice]")
{
    LardossaVoicePool pool;
    pool.prepare(44100.0, 512);

    const float ratios[4] = {1.f, 2.f, 3.f, 7.f};
    const float det[4] = {0.f, 0.f, 0.f, 0.f};
    const float lv[4] = {1.f, 0.8f, 0.5f, 0.3f};
    const float atk[4] = {2.f, 2.f, 2.f, 2.f};
    const float dec[4] = {100.f, 100.f, 100.f, 100.f};
    const float sus[4] = {0.8f, 0.8f, 0.8f, 0.8f};
    const float rel[4] = {200.f, 200.f, 200.f, 200.f};

    for (int k = 0; k < 10; ++k)
    {
        const int slot = pool.noteOn(48 + (k % 12), 1.f, true);
        REQUIRE(slot >= 1);
        pool.updateFmVoice(slot, 1, ratios, det, lv, 0.f, atk, dec, sus, rel);
    }

    std::vector<float> mono(4096, 0.f);
    for (int v = 1; v < LardossaVoicePool::kNumVoices; ++v)
        pool.renderFmVoiceAdding(v, mono.data(), 0, (int) mono.size());

    double energy = 0.0;
    for (float x : mono)
    {
        REQUIRE(std::isfinite(x));
        energy += (double) x * (double) x;
    }
    REQUIRE(energy > 1.0e-12);
}
