#include "AudioEngine.h"

#include <windows.h>
#include <mmsystem.h>

#include <iostream>

namespace {

const int BUFFER_COUNT = 4;
const int BUFFER_FRAMES = 768; // ~35 ms por buffer

struct Voice {
    const Sound* sound;
    std::size_t position;
    float volume;
    bool loop;
    bool active;
    int id;
};

}

struct AudioEngine::Impl {
    HWAVEOUT device = nullptr;
    HANDLE event = nullptr;
    HANDLE thread = nullptr;
    CRITICAL_SECTION lock;
    volatile LONG running = 0;

    WAVEHDR headers[BUFFER_COUNT];
    short buffers[BUFFER_COUNT][BUFFER_FRAMES];
    bool queued[BUFFER_COUNT];
    float mix[BUFFER_FRAMES];

    std::vector<Voice> voices;
    int nextId = 1;

    void fill(int index)
    {
        for (int i = 0; i < BUFFER_FRAMES; ++i) {
            mix[i] = 0.0f;
        }

        EnterCriticalSection(&lock);

        for (std::size_t v = 0; v < voices.size(); ++v) {
            Voice& voice = voices[v];
            const std::vector<float>& samples = voice.sound->samples;

            if (!voice.active || samples.empty()) {
                continue;
            }

            for (int i = 0; i < BUFFER_FRAMES; ++i) {
                if (voice.position >= samples.size()) {
                    if (!voice.loop) {
                        voice.active = false;
                        break;
                    }
                    voice.position = 0;
                }

                mix[i] += samples[voice.position++] * voice.volume;
            }
        }

        // Remove as vozes que terminaram
        for (std::size_t v = voices.size(); v > 0; --v) {
            if (!voices[v - 1].active) {
                voices.erase(voices.begin() + (v - 1));
            }
        }

        LeaveCriticalSection(&lock);

        for (int i = 0; i < BUFFER_FRAMES; ++i) {
            float sample = mix[i];

            // Limitador suave para evitar estalos quando muitos sons somam
            if (sample > 1.0f) sample = 1.0f;
            if (sample < -1.0f) sample = -1.0f;
            sample = sample * (1.5f - 0.5f * sample * sample);

            buffers[index][i] = static_cast<short>(sample * 32000.0f);
        }
    }

    void submit(int index)
    {
        fill(index);
        headers[index].dwFlags &= ~WHDR_DONE;
        waveOutWrite(device, &headers[index], sizeof(WAVEHDR));
        queued[index] = true;
    }

    static DWORD WINAPI threadMain(LPVOID parameter)
    {
        Impl* self = static_cast<Impl*>(parameter);

        for (int i = 0; i < BUFFER_COUNT; ++i) {
            self->submit(i);
        }

        while (InterlockedCompareExchange(&self->running, 1, 1) == 1) {
            WaitForSingleObject(self->event, 50);

            for (int i = 0; i < BUFFER_COUNT; ++i) {
                if (self->headers[i].dwFlags & WHDR_DONE) {
                    self->submit(i);
                }
            }
        }

        return 0;
    }
};

AudioEngine::AudioEngine()
    : m_impl(nullptr)
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::init()
{
    if (m_impl != nullptr) {
        return true;
    }

    Impl* impl = new Impl();
    InitializeCriticalSection(&impl->lock);
    impl->event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    WAVEFORMATEX format = {};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = SAMPLE_RATE;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 2;
    format.nAvgBytesPerSec = SAMPLE_RATE * 2;

    MMRESULT result = waveOutOpen(&impl->device, WAVE_MAPPER, &format,
                                  reinterpret_cast<DWORD_PTR>(impl->event), 0, CALLBACK_EVENT);

    if (result != MMSYSERR_NOERROR) {
        std::cerr << "Audio indisponivel (waveOutOpen falhou). O jogo segue sem som." << std::endl;
        CloseHandle(impl->event);
        DeleteCriticalSection(&impl->lock);
        delete impl;
        return false;
    }

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        ZeroMemory(&impl->headers[i], sizeof(WAVEHDR));
        impl->headers[i].lpData = reinterpret_cast<LPSTR>(impl->buffers[i]);
        impl->headers[i].dwBufferLength = BUFFER_FRAMES * sizeof(short);
        waveOutPrepareHeader(impl->device, &impl->headers[i], sizeof(WAVEHDR));
        impl->queued[i] = false;
    }

    impl->running = 1;
    impl->thread = CreateThread(nullptr, 0, Impl::threadMain, impl, 0, nullptr);
    SetThreadPriority(impl->thread, THREAD_PRIORITY_ABOVE_NORMAL);

    m_impl = impl;
    return true;
}

void AudioEngine::shutdown()
{
    if (m_impl == nullptr) {
        return;
    }

    InterlockedExchange(&m_impl->running, 0);
    SetEvent(m_impl->event);
    WaitForSingleObject(m_impl->thread, 2000);
    CloseHandle(m_impl->thread);

    waveOutReset(m_impl->device);

    for (int i = 0; i < BUFFER_COUNT; ++i) {
        waveOutUnprepareHeader(m_impl->device, &m_impl->headers[i], sizeof(WAVEHDR));
    }

    waveOutClose(m_impl->device);
    CloseHandle(m_impl->event);
    DeleteCriticalSection(&m_impl->lock);

    delete m_impl;
    m_impl = nullptr;
}

bool AudioEngine::isReady() const
{
    return m_impl != nullptr;
}

int AudioEngine::play(const Sound& sound, float volume, bool loop)
{
    if (m_impl == nullptr || sound.samples.empty()) {
        return -1;
    }

    EnterCriticalSection(&m_impl->lock);

    Voice voice = { &sound, 0, volume, loop, true, m_impl->nextId++ };
    m_impl->voices.push_back(voice);

    LeaveCriticalSection(&m_impl->lock);

    return voice.id;
}

void AudioEngine::setVolume(int voiceId, float volume)
{
    if (m_impl == nullptr) {
        return;
    }

    EnterCriticalSection(&m_impl->lock);

    for (std::size_t i = 0; i < m_impl->voices.size(); ++i) {
        if (m_impl->voices[i].id == voiceId) {
            m_impl->voices[i].volume = volume;
        }
    }

    LeaveCriticalSection(&m_impl->lock);
}

void AudioEngine::stop(int voiceId)
{
    if (m_impl == nullptr) {
        return;
    }

    EnterCriticalSection(&m_impl->lock);

    for (std::size_t i = 0; i < m_impl->voices.size(); ++i) {
        if (m_impl->voices[i].id == voiceId) {
            m_impl->voices[i].active = false;
        }
    }

    LeaveCriticalSection(&m_impl->lock);
}
