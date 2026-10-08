#include "SoundSynth.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace SoundSynth {

namespace {

const float PI = 3.14159265358979323846f;
const float RATE = static_cast<float>(AudioEngine::SAMPLE_RATE);

// Gerador de ruido deterministico (sempre gera os mesmos sons).
struct Noise {
    uint32_t state;

    explicit Noise(uint32_t seed) : state(seed * 2654435761u + 1u) {}

    float next()
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return (state & 0xFFFFFF) / static_cast<float>(0x800000) - 1.0f;
    }
};

// Filtro passa-baixa de um polo.
struct LowPass {
    float value = 0.0f;
    float amount;

    explicit LowPass(float a) : amount(a) {}

    float process(float input)
    {
        value += (input - value) * amount;
        return value;
    }
};

float midiToFrequency(int note)
{
    return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

void normalize(Sound& sound, float peak)
{
    float maximum = 0.0001f;
    for (float s : sound.samples) {
        maximum = std::fmax(maximum, std::fabs(s));
    }

    float gain = peak / maximum;
    for (float& s : sound.samples) {
        s *= gain;
    }
}

// Soma um som num buffer em loop (o final "dá a volta" para o inicio).
void addLooped(std::vector<float>& buffer, std::size_t start, const std::vector<float>& note)
{
    for (std::size_t i = 0; i < note.size(); ++i) {
        buffer[(start + i) % buffer.size()] += note[i];
    }
}

// ---------------------------------------------------------------- instrumentos

// Sanfona: harmonicos com leve vibrato e um "fole" (ataque e soltura suaves).
std::vector<float> accordion(float frequency, float seconds, float volume)
{
    std::size_t length = static_cast<std::size_t>((seconds + 0.15f) * RATE);
    std::vector<float> out(length);
    LowPass tone(0.35f);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float vibrato = 1.0f + 0.004f * std::sin(2.0f * PI * 5.5f * t);
        float phase = 2.0f * PI * frequency * vibrato * t;

        // Duas palhetas levemente desafinadas (o "chorus" da sanfona)
        float reedA = std::sin(phase) + 0.5f * std::sin(2.0f * phase) + 0.35f * std::sin(3.0f * phase) +
                      0.2f * std::sin(4.0f * phase) + 0.12f * std::sin(5.0f * phase);
        float reedB = std::sin(phase * 1.003f) + 0.4f * std::sin(2.0f * phase * 1.003f);

        float attack = std::fmin(t / 0.04f, 1.0f);
        float release = t > seconds ? std::exp(-(t - seconds) * 30.0f) : 1.0f;

        out[i] = tone.process((reedA + 0.6f * reedB) * 0.3f) * attack * release * volume;
    }

    return out;
}

std::vector<float> bass(float frequency, float seconds, float volume)
{
    std::size_t length = static_cast<std::size_t>((seconds + 0.2f) * RATE);
    std::vector<float> out(length);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float phase = 2.0f * PI * frequency * t;
        float envelope = std::exp(-t * 3.5f) * std::fmin(t / 0.01f, 1.0f);
        if (t > seconds) envelope *= std::exp(-(t - seconds) * 25.0f);

        out[i] = (std::sin(phase) + 0.25f * std::sin(2.0f * phase)) * envelope * volume;
    }

    return out;
}

std::vector<float> pad(const int notes[3], float seconds, float volume)
{
    std::size_t length = static_cast<std::size_t>((seconds + 0.6f) * RATE);
    std::vector<float> out(length, 0.0f);

    for (int n = 0; n < 3; ++n) {
        float frequency = midiToFrequency(notes[n]);

        for (std::size_t i = 0; i < length; ++i) {
            float t = i / RATE;
            float envelope = std::fmin(t / 0.35f, 1.0f);
            if (t > seconds) envelope *= std::exp(-(t - seconds) * 6.0f);

            float phase = 2.0f * PI * frequency * t;
            out[i] += (std::sin(phase) + 0.15f * std::sin(2.0f * phase)) * envelope * volume;
        }
    }

    return out;
}

// Zabumba: tambor grave com queda de afinacao.
std::vector<float> zabumba(float volume, uint32_t seed)
{
    std::size_t length = static_cast<std::size_t>(0.35f * RATE);
    std::vector<float> out(length);
    Noise noise(seed);
    float phase = 0.0f;

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float frequency = 55.0f + 70.0f * std::exp(-t * 30.0f);
        phase += 2.0f * PI * frequency / RATE;

        float body = std::sin(phase) * std::exp(-t * 9.0f);
        float skin = noise.next() * std::exp(-t * 60.0f) * 0.3f;

        out[i] = (body + skin) * volume;
    }

    return out;
}

// Triangulo: parciais metalicas inarmonicas.
std::vector<float> triangle(float volume, bool open)
{
    float decay = open ? 6.0f : 28.0f;
    std::size_t length = static_cast<std::size_t>((open ? 0.5f : 0.12f) * RATE);
    std::vector<float> out(length);

    const float partials[4] = { 3150.0f, 4410.0f, 5820.0f, 7300.0f };

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float sum = 0.0f;

        for (int p = 0; p < 4; ++p) {
            sum += std::sin(2.0f * PI * partials[p] * t) / (p + 1);
        }

        out[i] = sum * std::exp(-t * decay) * volume;
    }

    return out;
}

struct MelodyNote {
    float beat;
    int note;
    float length;
};

}

Sound musicMaceio()
{
    const float bpm = 96.0f;
    const float beat = 60.0f / bpm;
    const int bars = 8;

    Sound music;
    music.samples.assign(static_cast<std::size_t>(bars * 4 * beat * RATE), 0.0f);
    std::vector<float>& buffer = music.samples;

    auto at = [&](float beats) { return static_cast<std::size_t>(beats * beat * RATE); };

    // Progressao: C, Am, F, G, C, Am, Dm, G (raizes em MIDI)
    const int roots[bars] = { 48, 45, 41, 43, 48, 45, 38, 43 };
    const int chords[bars][3] = {
        { 60, 64, 67 }, { 57, 60, 64 }, { 53, 57, 60 }, { 55, 59, 62 },
        { 60, 64, 67 }, { 57, 60, 64 }, { 50, 53, 57 }, { 55, 59, 65 }
    };

    for (int bar = 0; bar < bars; ++bar) {
        float start = bar * 4.0f;

        addLooped(buffer, at(start), pad(chords[bar], 4.0f * beat, 0.035f));

        // Baixo e zabumba no ritmo do baiao: colcheia pontuada + semicolcheia + seminima
        for (int half = 0; half < 2; ++half) {
            float h = start + half * 2.0f;
            float root = midiToFrequency(roots[bar]);
            float fifth = midiToFrequency(roots[bar] + 7);

            addLooped(buffer, at(h), bass(root, 0.6f * beat, 0.32f));
            addLooped(buffer, at(h + 0.75f), bass(root, 0.2f * beat, 0.26f));
            addLooped(buffer, at(h + 1.0f), bass(fifth, 0.9f * beat, 0.28f));

            addLooped(buffer, at(h), zabumba(0.30f, bar * 4 + half));
            addLooped(buffer, at(h + 0.75f), zabumba(0.18f, bar * 4 + half + 100));
        }

        // Triangulo em colcheias, aberto no contratempo
        for (int eighth = 0; eighth < 8; ++eighth) {
            bool open = eighth % 2 == 1;
            addLooped(buffer, at(start + eighth * 0.5f), triangle(open ? 0.030f : 0.020f, open));
        }
    }

    // Melodia da sanfona (oito compassos)
    const MelodyNote melody[] = {
        { 0.0f, 76, 1.0f }, { 1.0f, 79, 0.5f }, { 1.5f, 81, 0.5f }, { 2.0f, 79, 1.0f }, { 3.0f, 76, 1.0f },
        { 4.0f, 72, 1.0f }, { 5.0f, 76, 1.0f }, { 6.0f, 74, 1.5f }, { 7.5f, 72, 0.5f },
        { 8.0f, 69, 1.0f }, { 9.0f, 72, 0.5f }, { 9.5f, 74, 0.5f }, { 10.0f, 72, 1.0f }, { 11.0f, 69, 1.0f },
        { 12.0f, 71, 1.0f }, { 13.0f, 74, 1.0f }, { 14.0f, 77, 1.0f }, { 15.0f, 74, 1.0f },
        { 16.0f, 79, 0.5f }, { 16.5f, 76, 0.5f }, { 17.0f, 79, 1.0f }, { 18.0f, 84, 1.5f }, { 19.5f, 83, 0.5f },
        { 20.0f, 81, 1.0f }, { 21.0f, 79, 0.5f }, { 21.5f, 76, 0.5f }, { 22.0f, 72, 2.0f },
        { 24.0f, 74, 1.0f }, { 25.0f, 77, 1.0f }, { 26.0f, 81, 1.0f }, { 27.0f, 79, 1.0f },
        { 28.0f, 77, 0.5f }, { 28.5f, 76, 0.5f }, { 29.0f, 74, 1.0f }, { 30.0f, 71, 1.0f }, { 31.0f, 74, 1.0f }
    };

    for (const MelodyNote& n : melody) {
        addLooped(buffer, at(n.beat), accordion(midiToFrequency(n.note), n.length * beat * 0.92f, 0.11f));
    }

    normalize(music, 0.8f);
    return music;
}

Sound waves()
{
    const float seconds = 16.0f;
    const float fade = 1.0f;
    std::size_t length = static_cast<std::size_t>(seconds * RATE);
    std::size_t fadeLength = static_cast<std::size_t>(fade * RATE);

    // Gera um pouco a mais e funde o final com o inicio para o loop nao "estalar"
    std::vector<float> raw(length + fadeLength);
    Noise noise(77);
    LowPass rumble(0.02f);
    LowPass wash(0.25f);
    LowPass washSlow(0.05f);

    for (std::size_t i = 0; i < raw.size(); ++i) {
        float t = i / RATE;
        float n = noise.next();

        float low = rumble.process(n);
        float band = wash.process(n) - washSlow.process(n);

        // Duas ondas por loop: sobe, quebra e recua
        float swell = 0.5f - 0.5f * std::cos(2.0f * PI * t / 8.0f);
        float crash = std::pow(swell, 3.0f);

        raw[i] = low * (0.8f + 0.6f * swell) * 2.5f + band * (0.15f + 1.1f * crash);
    }

    Sound sound;
    sound.samples.assign(raw.begin(), raw.begin() + length);

    for (std::size_t i = 0; i < fadeLength; ++i) {
        float k = static_cast<float>(i) / fadeLength;
        sound.samples[i] = sound.samples[i] * k + raw[length + i] * (1.0f - k);
    }

    normalize(sound, 0.7f);
    return sound;
}

Sound footstepSand(int variation)
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(0.14f * RATE);
    sound.samples.resize(length);

    Noise noise(200 + variation);
    LowPass high(0.6f);
    LowPass low(0.08f);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float n = noise.next();
        float crunch = high.process(n) - low.process(n);
        float grains = (noise.next() > 0.92f) ? noise.next() * 0.6f : 0.0f;
        float envelope = std::fmin(t / 0.006f, 1.0f) * std::exp(-t * 32.0f);

        sound.samples[i] = (crunch + grains) * envelope;
    }

    normalize(sound, 0.55f);
    return sound;
}

Sound footstepWood(int variation)
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(0.2f * RATE);
    sound.samples.resize(length);

    Noise noise(300 + variation);
    float pitch = 1.0f + 0.06f * variation;
    float phase1 = 0.0f;
    float phase2 = 0.0f;

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        phase1 += 2.0f * PI * 165.0f * pitch * (1.0f + 0.4f * std::exp(-t * 60.0f)) / RATE;
        phase2 += 2.0f * PI * 410.0f * pitch / RATE;

        float knock = std::sin(phase1) * std::exp(-t * 28.0f) + 0.4f * std::sin(phase2) * std::exp(-t * 45.0f);
        float click = noise.next() * std::exp(-t * 300.0f) * 0.5f;

        sound.samples[i] = knock + click;
    }

    normalize(sound, 0.6f);
    return sound;
}

Sound footstepStone(int variation)
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(0.1f * RATE);
    sound.samples.resize(length);

    Noise noise(400 + variation);
    LowPass low(0.15f);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float n = noise.next();
        float tap = (n - low.process(n)) * std::exp(-t * 70.0f);
        float body = std::sin(2.0f * PI * 220.0f * t) * std::exp(-t * 50.0f) * 0.4f;

        sound.samples[i] = tap + body;
    }

    normalize(sound, 0.45f);
    return sound;
}

Sound chime()
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(1.4f * RATE);
    sound.samples.assign(length, 0.0f);

    const float notes[2] = { 1318.5f, 1975.5f }; // mi e si
    const float starts[2] = { 0.0f, 0.12f };

    for (int n = 0; n < 2; ++n) {
        for (std::size_t i = static_cast<std::size_t>(starts[n] * RATE); i < length; ++i) {
            float t = i / RATE - starts[n];
            float phase = 2.0f * PI * notes[n] * t;
            float envelope = std::fmin(t / 0.003f, 1.0f) * std::exp(-t * 4.5f);

            sound.samples[i] += (std::sin(phase) + 0.3f * std::sin(2.01f * phase) + 0.1f * std::sin(3.0f * phase)) * envelope;
        }
    }

    normalize(sound, 0.5f);
    return sound;
}

Sound leavesRustle()
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(1.1f * RATE);
    sound.samples.resize(length);

    Noise noise(500);
    Noise flutter(501);
    LowPass high(0.7f);
    LowPass low(0.12f);
    LowPass gust(0.002f);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float n = noise.next();
        float hiss = high.process(n) - low.process(n);
        float wobble = 0.6f + 0.4f * std::fabs(gust.process(flutter.next()) * 40.0f);
        float envelope = std::fmin(t / 0.05f, 1.0f) * std::exp(-t * 3.0f);

        float thump = std::sin(2.0f * PI * 90.0f * t) * std::exp(-t * 25.0f) * 0.8f;

        sound.samples[i] = hiss * wobble * envelope + thump;
    }

    normalize(sound, 0.5f);
    return sound;
}

Sound dialogueBlip()
{
    Sound sound;
    std::size_t length = static_cast<std::size_t>(0.07f * RATE);
    sound.samples.resize(length);

    for (std::size_t i = 0; i < length; ++i) {
        float t = i / RATE;
        float envelope = std::fmin(t / 0.003f, 1.0f) * std::exp(-t * 60.0f);
        sound.samples[i] = (std::sin(2.0f * PI * 990.0f * t) + 0.3f * std::sin(2.0f * PI * 1980.0f * t)) * envelope;
    }

    normalize(sound, 0.4f);
    return sound;
}

}
