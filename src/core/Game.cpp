#include "Game.h"

#include "Input.h"
#include "graphics/GLFunctions.h"

#include <iostream>

namespace {

// Ilha de areia no centro: um cubo achatado com o topo em y = ISLAND_TOP.
const float ISLAND_SIZE = 14.0f;
const float ISLAND_TOP = 0.1f;

}

Game::Game()
    : m_window(1280, 720, "Rota das Aguas - Uma Aventura Matematica"),
      m_cube(Mesh::createCube()),
      m_water(Mesh::createWaterGrid(60, 1.5f)),
      m_player(m_cube)
{
}

bool Game::init()
{
    if (!m_window.init()) {
        return false;
    }

    if (!GL2::load()) {
        std::cerr << "O driver de video nao oferece OpenGL 2.0 (shaders)." << std::endl;
        return false;
    }

    if (!m_renderer.init()) {
        return false;
    }

    Input::update(m_window.getNativeWindow());

    m_boat.load();
    m_boat.setPosition(10.0f, 0.25f, -4.0f);

    m_player.setGroundHeight(ISLAND_TOP);

    m_camera.setAspect(m_window.getAspect());
    m_camera.snapTo(m_player.getPosition(), m_player.getYaw());

    return true;
}

void Game::run()
{
    double previousTime = glfwGetTime();

    while (!m_window.shouldClose()) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - previousTime);
        previousTime = currentTime;

        // Evita movimentos gigantescos caso a aplicacao congele por algum motivo.
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }

        update(deltaTime);
        render();

        m_window.swapBuffers();
        m_window.pollEvents();
    }
}

void Game::update(float deltaTime)
{
    if (Input::isKeyDown(GLFW_KEY_ESCAPE)) {
        m_window.close();
    }

    m_player.update(deltaTime);
    m_boat.update(deltaTime);

    m_camera.setAspect(m_window.getAspect());
    m_camera.follow(m_player.getPosition(), m_player.getYaw(), deltaTime);
}

void Game::render()
{
    m_renderer.beginFrame(m_camera);

    float identity[16];
    MathUtils::identity(identity);
    m_renderer.draw(m_water, identity, { 1.0f, 1.0f, 1.0f });

    Transform island;
    island.positionY = ISLAND_TOP - 0.25f;
    island.scale(ISLAND_SIZE, 0.5f, ISLAND_SIZE);

    float islandMatrix[16];
    island.getMatrix(islandMatrix);
    m_renderer.draw(m_cube, islandMatrix, { 0.93f, 0.84f, 0.62f });

    m_boat.draw(m_renderer);
    m_player.draw(m_renderer);
}
