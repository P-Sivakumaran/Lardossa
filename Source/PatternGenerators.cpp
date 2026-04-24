#include "PatternGenerators.h"
#include <JuceHeader.h>

namespace patterns
{

void clearSequencer(SequencerStepArray& steps)
{
    for (auto& s : steps)
    {
        s.enabled = false;
        s.accent = false;
        s.slide = false;
        s.tie = false;
        s.lengthFrac = 1.0f;
    }
}

void generateEbmBassPattern(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety)
{
    std::uniform_int_distribution<int> d6(0, 5);
    std::uniform_int_distribution<int> d3(0, 2);
    std::uniform_int_distribution<int> d2(0, 1);
    std::uniform_real_distribution<float> uf(0.0f, 1.0f);

    const int minorPent[] = {0, 3, 5, 7, 10, 12, 15, 17};
    const int phryg[] = {0, 1, 3, 5, 7, 8, 10, 12, 13, 15};
    const int dorian[] = {0, 2, 3, 5, 7, 9, 10, 12, 14, 15};

    const int* scale = minorPent;
    int scaleN = 8;
    switch (d3(rng))
    {
        case 0:
            scale = minorPent;
            scaleN = 8;
            break;
        case 1:
            scale = phryg;
            scaleN = 10;
            break;
        default:
            scale = dorian;
            scaleN = 10;
            break;
    }

    const int root = 42 + d3(rng);
    const float restBias = variety * 0.45f;
    const float slideBias = 0.15f + variety * 0.55f;
    const float tieBias = 0.08f + variety * 0.35f;

    int lastNote = root;

    for (int i = 0; i < seqLen; ++i)
    {
        auto& step = steps[(size_t) i];
        const int pos = i % 16;

        if (pos % 4 == 0)
        {
            step.enabled = true;
            step.accent = (pos == 0 || pos == 8 || (pos == 4 && uf(rng) < 0.35f + variety * 0.3f));
        }
        else if (pos % 2 == 0)
        {
            const float pOn = 0.62f - restBias * 0.35f;
            step.enabled = (uf(rng) < pOn);
            step.accent = (uf(rng) < 0.12f + variety * 0.2f);
        }
        else
        {
            const float p16 = 0.12f + variety * 0.38f;
            step.enabled = (uf(rng) < p16);
            step.accent = false;
        }

        if (step.enabled)
        {
            const auto roll = d6(rng);
            std::uniform_int_distribution<int> deg(0, scaleN - 1);
            int interval = scale[deg(rng)];
            if (roll < 2)
                interval = 0;
            else if (roll < 4)
                interval = scale[juce::jlimit(1, scaleN - 1, 1 + d3(rng))];

            step.note = juce::jlimit(28, 76, root + interval);

            if (uf(rng) < 0.18f + variety * 0.25f)
                step.note = juce::jlimit(28, 76, step.note + (d2(rng) == 0 ? 12 : -12));

            if (std::abs(step.note - lastNote) > 8 && uf(rng) < 0.65f)
                step.note = juce::jlimit(28, 76, lastNote + (step.note > lastNote ? 2 : -2));

            lastNote = step.note;
            step.velocity = step.accent ? (0.92f + uf(rng) * 0.06f) : (0.62f + uf(rng) * 0.22f);

            step.slide = (uf(rng) < slideBias) && (pos % 2 == 1 || pos % 4 == 3 || uf(rng) < 0.3f);
            step.tie = (uf(rng) < tieBias) && (pos % 4 == 0 || pos % 4 == 2);

            static constexpr float lenOpts[] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f};
            step.lengthFrac = 1.0f;
            if (uf(rng) < 0.12f + variety * 0.38f)
                step.lengthFrac = lenOpts[std::uniform_int_distribution<int>(0, 4)(rng)];
        }
        else
        {
            step.note = root;
            step.velocity = 0.85f;
            step.slide = false;
            step.tie = false;
            step.lengthFrac = 1.0f;
        }
    }
}

void generateDetroitBass(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety)
{
    std::uniform_real_distribution<float> uf(0.0f, 1.0f);
    const int root = 40;
    const int biased[] = {0, 2, 4, 8, 12};
    const int nBias = 5;

    for (int i = 0; i < seqLen; ++i)
    {
        auto& st = steps[(size_t) i];
        const int pos = i % 16;

        if (pos == 0 || pos == 4)
        {
            st.enabled = true;
            st.note = root + (pos == 4 ? 5 : 0);
            st.velocity = 0.88f;
            st.accent = (pos == 0);
            st.slide = false;
            st.tie = (variety > 0.4f && uf(rng) < variety * 0.35f);
            st.lengthFrac = (pos == 0 && uf(rng) < variety) ? 0.5f : 1.0f;
            continue;
        }

        bool onBiased = false;
        for (int b = 0; b < nBias; ++b)
            if (biased[b] == pos)
            {
                onBiased = true;
                break;
            }

        const float p = onBiased ? (0.55f + variety * 0.35f) : (0.25f + variety * 0.25f);
        st.enabled = uf(rng) < p;

        if (!st.enabled)
        {
            st.note = root;
            st.velocity = 0.5f;
            st.tie = (uf(rng) < variety * 0.4f);
            continue;
        }

        st.note = root + (uf(rng) < 0.7f ? 0 : (uf(rng) < 0.5f ? 7 : 12));
        st.velocity = 0.72f + uf(rng) * 0.2f;
        st.accent = (pos % 8 == 0);
        st.slide = (pos % 2 == 1 && uf(rng) < 0.25f + variety * 0.4f);
        st.tie = (uf(rng) < variety * 0.45f);
        st.lengthFrac = uf(rng) < 0.15f + variety * 0.25f ? 0.5f : 1.0f;
    }
}

void generateDetroitLead(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety)
{
    std::uniform_real_distribution<float> uf(0.0f, 1.0f);
    const int root = 64;
    for (int i = 0; i < seqLen; ++i)
    {
        auto& st = steps[(size_t) i];
        const float sparsity = 0.35f + (1.f - variety) * 0.35f;
        st.enabled = uf(rng) > sparsity;
        if (!st.enabled)
        {
            st.note = root;
            st.velocity = 0.4f;
            st.tie = false;
            st.slide = false;
            st.lengthFrac = 1.0f;
            continue;
        }
        const int deg[] = {0, 2, 4, 5, 7, 9, 11, 12};
        st.note = juce::jlimit(60, 96, root + deg[std::uniform_int_distribution<int>(0, 7)(rng)]);
        st.velocity = 0.55f + uf(rng) * 0.35f;
        st.accent = uf(rng) < 0.2f;
        st.slide = uf(rng) < 0.35f + variety * 0.2f;
        st.tie = uf(rng) < 0.25f + variety * 0.45f;
        st.lengthFrac = uf(rng) < 0.2f + variety * 0.3f ? 2.0f : 1.0f;
    }
}

void generateElectroGroove(SequencerStepArray& steps, std::mt19937& rng, int seqLen, float variety)
{
    std::uniform_real_distribution<float> uf(0.0f, 1.0f);
    const int root = 45;
    for (int i = 0; i < seqLen; ++i)
    {
        auto& st = steps[(size_t) i];
        const int pos = i % 16;

        if (pos == 0)
        {
            st.enabled = true;
            st.note = root;
            st.velocity = 0.95f;
            st.accent = true;
            st.slide = false;
            st.tie = false;
            st.lengthFrac = 1.0f;
            continue;
        }

        if (pos == 4 || pos == 12)
        {
            st.enabled = true;
            st.note = root - 5;
            st.velocity = 0.9f;
            st.accent = true;
            st.slide = false;
            st.tie = false;
            st.lengthFrac = 1.0f;
            continue;
        }

        const float ghost = variety * 0.35f;
        if (pos % 2 == 0)
            st.enabled = uf(rng) < (0.45f + ghost);
        else
            st.enabled = uf(rng) < (0.12f + ghost);

        if (!st.enabled)
        {
            st.note = root;
            st.velocity = 0.5f;
            continue;
        }

        st.note = root + (uf(rng) < 0.85f ? 0 : 12);
        st.velocity = 0.65f + uf(rng) * 0.25f;
        st.accent = false;
        st.slide = (pos % 4 == 3 && uf(rng) < 0.3f);
        st.tie = false;
        st.lengthFrac = 1.0f;
    }
}

} // namespace patterns
