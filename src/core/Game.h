#ifndef GAME_H
#define GAME_H

// Loop principal do jogo: cria a janela, carrega a cena e, a cada quadro,
// le teclado e mouse, atualiza a Lia, a camera, a cena e o audio, e desenha.

#include "Window.h"
#include "audio/AudioEngine.h"
#include "entities/Player.h"
#include "graphics/Camera.h"
#include "graphics/Renderer.h"
#include "scenes/Maceio/MaceioScene.h"
#include "ui/TextRenderer.h"

#include <string>

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();

private:
    Window m_window;
    Renderer m_renderer;
    Camera m_camera;
    AudioEngine m_audio;

    MaceioScene m_maceio;
    Scene* m_scene;
    Player m_player;

    // Sons gerados por codigo
    Sound m_music;
    Sound m_waves;
    Sound m_stepsSand[4];
    Sound m_stepsWood[4];
    Sound m_stepsStone[4];
    Sound m_chime;
    Sound m_leaves;

    int m_musicVoice;
    int m_wavesVoice;
    bool m_musicOn;
    int m_stepCounter;

    float m_time;
    float m_dragDistance;
    float m_messageTimer;
    std::string m_message;
    TextRenderer m_text;

    void loadSounds();
    void update(float deltaTime);
    void handleCamera();
    void handleInteraction();
    void handleBoatControls(float deltaTime);
    void render();
    void renderInterface();
};

#endif
