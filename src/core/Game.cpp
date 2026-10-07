#include "Game.h"

#include "Input.h"
#include "audio/SoundSynth.h"
#include "graphics/GLFunctions.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace MathUtils;

namespace {

const char* TITLE = "Rota das Águas - Maceió";
const char* HINT = "WASD/setas: andar | Shift: correr | arrastar o mouse: câmera | roda: zoom | clique ou E: interagir | M: música";

const float MUSIC_VOLUME = 0.28f;

// Salva o conteudo atual da tela (framebuffer) num arquivo .bmp de 24 bits.
void saveScreenshot(const char* path, int width, int height)
{
    int rowSize = (width * 3 + 3) & ~3;
    std::vector<unsigned char> pixels(rowSize * height);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* p = &pixels[y * rowSize + x * 3];
            unsigned char red = p[0];
            p[0] = p[2];
            p[2] = red;
        }
    }

    unsigned char header[54] = { 'B', 'M' };
    auto write32 = [&](int offset, int value) {
        header[offset] = value & 0xFF;
        header[offset + 1] = (value >> 8) & 0xFF;
        header[offset + 2] = (value >> 16) & 0xFF;
        header[offset + 3] = (value >> 24) & 0xFF;
    };
    write32(2, 54 + static_cast<int>(pixels.size()));
    write32(10, 54);
    write32(14, 40);
    write32(18, width);
    write32(22, height);
    header[26] = 1;
    header[28] = 24;
    write32(34, static_cast<int>(pixels.size()));

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(header), 54);
    file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    std::cout << "Captura salva em " << path << std::endl;
}
const float MESSAGE_SECONDS = 6.0f;

}

Game::Game()
    : m_window(1280, 720, TITLE),
      m_scene(&m_maceio),
      m_musicVoice(-1),
      m_wavesVoice(-1),
      m_musicOn(true),
      m_stepCounter(0),
      m_time(0.0f),
      m_dragDistance(0.0f),
      m_messageTimer(0.0f)
{
    m_baseTitle = std::string(TITLE) + "  |  " + HINT;
}

Game::~Game()
{
    m_audio.shutdown();
}

void Game::loadSounds()
{
    m_music = SoundSynth::musicMaceio();
    m_waves = SoundSynth::waves();

    for (int i = 0; i < 4; ++i) {
        m_stepsSand[i] = SoundSynth::footstepSand(i);
        m_stepsWood[i] = SoundSynth::footstepWood(i);
        m_stepsStone[i] = SoundSynth::footstepStone(i);
    }

    m_chime = SoundSynth::chime();
    m_leaves = SoundSynth::leavesRustle();
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

    m_window.setTitle(m_baseTitle);
    Input::init(m_window.getNativeWindow());

    if (!m_scene->init()) {
        return false;
    }

    m_player.build();
    m_player.placeAt(m_scene->getPlayerStart(), 0.0f);
    float cameraYaw = 0.0f;

    // Para testes: ROTA_START="x z giroDaLia giroDaCamera" posiciona a Lia e a camera
    if (const char* start = std::getenv("ROTA_START")) {
        float x = 0.0f, z = 0.0f, yaw = 0.0f;
        if (std::sscanf(start, "%f %f %f %f", &x, &z, &yaw, &cameraYaw) >= 2) {
            m_player.placeAt({ x, m_scene->groundHeight(x, z), z }, yaw);
        }
    }

    m_camera.setAspect(m_window.getAspect());
    m_camera.snapTo(m_player.getPosition(), cameraYaw);

    loadSounds();

    if (m_audio.init()) {
        m_musicVoice = m_audio.play(m_music, MUSIC_VOLUME, true);
        m_wavesVoice = m_audio.play(m_waves, 0.3f, true);
    }

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

        // F12 salva uma captura de tela; ROTA_AUTOSHOT=segundos salva uma e fecha (testes)
        const char* autoShot = std::getenv("ROTA_AUTOSHOT");
        if (Input::wasKeyPressed(GLFW_KEY_F12)) {
            saveScreenshot("captura.bmp", m_window.getWidth(), m_window.getHeight());
        }
        if (autoShot != nullptr && m_time > static_cast<float>(std::atof(autoShot))) {
            saveScreenshot("autoshot.bmp", m_window.getWidth(), m_window.getHeight());
            m_window.close();
        }

        m_window.swapBuffers();
        m_window.pollEvents();
    }
}

void Game::handleCamera()
{
    // Arrastar com o botao esquerdo ou direito gira a camera; a roda aproxima/afasta
    bool dragging = Input::isMouseDown(GLFW_MOUSE_BUTTON_LEFT) || Input::isMouseDown(GLFW_MOUSE_BUTTON_RIGHT);
    float dx = static_cast<float>(Input::mouseDeltaX());
    float dy = static_cast<float>(Input::mouseDeltaY());

    if (Input::wasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_dragDistance = 0.0f;
    }

    if (dragging) {
        m_camera.orbit(-dx * 0.3f, dy * 0.2f);
        m_dragDistance += std::fabs(dx) + std::fabs(dy);
    }

    if (Input::scrollDelta() != 0.0) {
        m_camera.zoom(static_cast<float>(Input::scrollDelta()));
    }
}

void Game::handleInteraction()
{
    Vec3 playerPosition = m_player.getPosition();

    // Objeto em foco: o mais proximo ao alcance da Lia (fica brilhando)
    int focused = m_scene->findInteractable(playerPosition, nullptr, nullptr);
    m_scene->setFocus(focused);

    int target = -1;

    if (Input::wasKeyPressed(GLFW_KEY_E)) {
        target = focused;
    }

    // Clique (sem arrastar): interage com o objeto clicado, ou com o que esta em foco
    if (Input::wasMouseReleased(GLFW_MOUSE_BUTTON_LEFT) && m_dragDistance < 6.0f) {
        int width = 1;
        int height = 1;
        glfwGetWindowSize(m_window.getNativeWindow(), &width, &height);

        float ndcX = static_cast<float>(2.0 * Input::mouseX() / width - 1.0);
        float ndcY = static_cast<float>(1.0 - 2.0 * Input::mouseY() / height);

        Vec3 origin;
        Vec3 direction;
        m_camera.screenRay(ndcX, ndcY, origin, direction);

        target = m_scene->findInteractable(playerPosition, &origin, &direction);
        if (target < 0) {
            target = focused;
        }
    }

    if (target < 0) {
        return;
    }

    Interaction result = m_scene->interact(target);
    m_audio.play(result.sound == InteractionSound::Leaves ? m_leaves : m_chime, 0.55f, false);

    if (result.sound == InteractionSound::Leaves) {
        m_audio.play(m_chime, 0.25f, false);
    }

    m_window.setTitle(std::string(TITLE) + "  |  " + result.message);
    m_messageTimer = MESSAGE_SECONDS;
}

void Game::handleBoatControls(float deltaTime)
{
    // Transformacoes interativas do barco: R/T giram, Z/X mudam a escala
    Transform& boat = m_maceio.getBoat().getTransform();

    const float rotationSpeed = 40.0f * deltaTime;
    const float scaleFactor = 1.0f + 0.5f * deltaTime;

    if (Input::isKeyDown(GLFW_KEY_R)) boat.rotate(0.0f, rotationSpeed, 0.0f);
    if (Input::isKeyDown(GLFW_KEY_T)) boat.rotate(0.0f, -rotationSpeed, 0.0f);

    if (Input::isKeyDown(GLFW_KEY_Z) && boat.scaleX > 1.2f) {
        boat.scale(1.0f / scaleFactor, 1.0f / scaleFactor, 1.0f / scaleFactor);
    }
    if (Input::isKeyDown(GLFW_KEY_X) && boat.scaleX < 3.0f) {
        boat.scale(scaleFactor, scaleFactor, scaleFactor);
    }
}

void Game::update(float deltaTime)
{
    m_time += deltaTime;

    // No modo de captura automatica o teclado e o mouse sao ignorados
    if (std::getenv("ROTA_AUTOSHOT") == nullptr) {
        Input::update();
    }

    if (Input::isKeyDown(GLFW_KEY_ESCAPE)) {
        m_window.close();
    }

    if (Input::wasKeyPressed(GLFW_KEY_M)) {
        m_musicOn = !m_musicOn;
        m_audio.setVolume(m_musicVoice, m_musicOn ? MUSIC_VOLUME : 0.0f);
    }

    handleCamera();
    handleBoatControls(deltaTime);

    // Movimento relativo a camera: W anda para onde a camera olha
    Vec3 forward = forwardFromYaw(m_camera.getYaw());
    Vec3 right = { -forward.z, 0.0f, forward.x };
    Vec3 move = { 0.0f, 0.0f, 0.0f };

    if (Input::isKeyDown(GLFW_KEY_W) || Input::isKeyDown(GLFW_KEY_UP))    move = add(move, forward);
    if (Input::isKeyDown(GLFW_KEY_S) || Input::isKeyDown(GLFW_KEY_DOWN))  move = subtract(move, forward);
    if (Input::isKeyDown(GLFW_KEY_D) || Input::isKeyDown(GLFW_KEY_RIGHT)) move = add(move, right);
    if (Input::isKeyDown(GLFW_KEY_A) || Input::isKeyDown(GLFW_KEY_LEFT))  move = subtract(move, right);

    bool running = Input::isKeyDown(GLFW_KEY_LEFT_SHIFT) || Input::isKeyDown(GLFW_KEY_RIGHT_SHIFT);

    m_player.update(deltaTime, normalize(move), running, *m_scene);

    Vec3 position = m_player.getPosition();
    m_scene->update(deltaTime, m_time, position);

    handleInteraction();

    // Sons de passos conforme o piso
    if (m_player.tookStep()) {
        int variation = (m_stepCounter++) % 4;
        Surface surface = m_scene->surfaceAt(position.x, position.z);
        const Sound& step = surface == Surface::Wood ? m_stepsWood[variation]
                          : surface == Surface::Stone ? m_stepsStone[variation]
                          : m_stepsSand[variation];
        m_audio.play(step, running ? 0.45f : 0.32f, false);
    }

    // Ondas mais altas perto do mar
    float closeness = clamp(1.0f - m_scene->distanceToSea(position) / 30.0f, 0.0f, 1.0f);
    m_audio.setVolume(m_wavesVoice, 0.12f + 0.5f * closeness);

    if (m_messageTimer > 0.0f) {
        m_messageTimer -= deltaTime;
        if (m_messageTimer <= 0.0f) {
            m_window.setTitle(m_baseTitle);
        }
    }

    const Scene& scene = *m_scene;
    m_camera.setAspect(m_window.getAspect());
    m_camera.update(position, deltaTime, [&scene](float x, float z) { return scene.groundHeight(x, z); });
}

void Game::render()
{
    m_renderer.beginFrame(m_camera, m_scene->getLight(), m_time);

    m_scene->drawSky(m_renderer, m_camera, m_time);
    m_scene->drawOpaque(m_renderer, m_time);
    m_player.draw(m_renderer, m_time);

    m_renderer.beginTransparent();
    m_scene->drawShadows(m_renderer, m_player.getPosition());
    m_scene->drawWater(m_renderer);
    m_renderer.endTransparent();
}
