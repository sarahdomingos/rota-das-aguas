#ifndef GAME_H
#define GAME_H

// Loop principal do jogo: cria a janela, carrega os recursos e,
// a cada quadro, le a entrada, atualiza as entidades e desenha a cena.

#include "Window.h"
#include "entities/Boat.h"
#include "entities/Player.h"
#include "graphics/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Renderer.h"

class Game {
public:
    Game();

    bool init();
    void run();

private:
    Window m_window;
    Renderer m_renderer;
    Camera m_camera;

    Mesh m_cube;
    Mesh m_water;

    Player m_player;
    Boat m_boat;

    void update(float deltaTime);
    void render();
};

#endif
