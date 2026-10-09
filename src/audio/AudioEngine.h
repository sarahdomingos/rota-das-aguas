#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

// Mixer de audio simples sobre a API waveOut do Windows (winmm), que ja vem
// com o MinGW: toca varios sons ao mesmo tempo (musica em loop + efeitos).
// O mixer roda numa thread do Win32 (o MinGW 32-bit usado nao tem std::thread).

#include <vector>

// Som mono em ponto flutuante (-1 a 1), na taxa AudioEngine::SAMPLE_RATE.
struct Sound {
    std::vector<float> samples;
};

class AudioEngine {
public:
    static const int SAMPLE_RATE = 22050;

    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool init();
    void shutdown();
    bool isReady() const;

    // Retorna um identificador da "voz" (ou -1 se o audio nao estiver ativo).
    int play(const Sound& sound, float volume, bool loop);
    void setVolume(int voice, float volume);
    void stop(int voice);

private:
    struct Impl;
    Impl* m_impl;
};

#endif
