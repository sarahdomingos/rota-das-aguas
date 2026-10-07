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
#include "gameplay/Dialogue.h"
#include "ui/TextRenderer.h"

#include <string>

// Estados do jogo. So JOGANDO aceita movimento e interacao; DIALOGO e DESAFIO
// congelam a Lia mas a cena continua animando; PAUSA congela o mundo inteiro;
// TRANSICAO escurece a tela e volta (sera usada na troca de cena).
enum class GameState {
    Playing,
    Dialogue,
    Challenge,
    Paused,
    Transition
};

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
    Sound m_blip;

    int m_musicVoice;
    int m_wavesVoice;
    bool m_musicOn;
    int m_stepCounter;

    float m_time;
    float m_dragDistance;
    std::string m_message;

    GameState m_state;
    GameState m_stateBeforePause;
    float m_transitionTimer;
    TextRenderer m_text;
    Dialogue m_dialogue;

    void changeState(GameState next);
    void enterState(GameState state);
    void exitState(GameState state);
    void openDialogue(const std::string& id);

    void loadSounds();
    void update(float deltaTime);
    void handleCamera();
    void handleInteraction();
    void handleMovement(float deltaTime, bool allowMove);
    void updateWorld(float deltaTime);
    void handleBoatControls(float deltaTime);
    void render();
    void renderInterface();
};

#endif
