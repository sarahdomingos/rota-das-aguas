#include "Game.h"

#include "AssetPath.h"
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
const char* HINT = "WASD: andar  ·  Shift: correr  ·  arrastar o mouse: câmera  ·  roda: zoom  ·  clique ou E: interagir  ·  M: música  ·  Esc/P: pausa";

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
// Duracao total da transicao (metade escurecendo, metade clareando)
const float TRANSITION_SECONDS = 1.2f;

const char* PAUSE_LINES[] = {
    "WASD ou setas: andar  ·  Shift: correr",
    "Arrastar o mouse: girar a câmera  ·  Roda: zoom",
    "Clique ou E: interagir  ·  E, Espaço ou clique: fechar mensagem",
    "R / T: girar o barco  ·  Z / X: tamanho do barco",
    "M: música  ·  F12: captura de tela",
    "F2: teste de transição  ·  F3: teste de desafio"
};

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
      m_state(GameState::Playing),
      m_stateBeforePause(GameState::Playing),
      m_transitionTimer(0.0f)
{
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
    m_blip = SoundSynth::dialogueBlip();
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

    if (!m_text.init()) {
        std::cerr << "Nao foi possivel criar a fonte do texto na tela." << std::endl;
    }

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

    m_dialogue.loadFile(AssetPath::resolve("assets/dialogos/maceio.txt"));
    openDialogue("inicio");

    // Para testes: ROTA_TEST_STATE=pausa abre o jogo ja na tela de pausa
    const char* testState = std::getenv("ROTA_TEST_STATE");
    if (testState != nullptr && std::string(testState) == "pausa") {
        m_stateBeforePause = m_state;
        changeState(GameState::Paused);
    }

    return true;
}

void Game::run()
{
    double previousTime = glfwGetTime();
    double startTime = previousTime;

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
        if (autoShot != nullptr && currentTime - startTime > std::atof(autoShot)) {
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

    openDialogue(result.dialogueId);
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

// ------------------------------------------------------------------ estados

void Game::changeState(GameState next)
{
    if (next == m_state) {
        return;
    }

    exitState(m_state);
    m_state = next;
    enterState(next);
}

void Game::enterState(GameState state)
{
    switch (state) {
    case GameState::Paused:
        // Musica mais baixa enquanto o jogo esta pausado
        m_audio.setVolume(m_musicVoice, m_musicOn ? MUSIC_VOLUME * 0.35f : 0.0f);
        break;
    case GameState::Transition:
        m_transitionTimer = 0.0f;
        break;
    case GameState::Challenge:
        m_message = "Desafio (teste): aqui vão aparecer os desafios de matemática.";
        break;
    default:
        break;
    }
}

void Game::exitState(GameState state)
{
    switch (state) {
    case GameState::Paused:
        m_audio.setVolume(m_musicVoice, m_musicOn ? MUSIC_VOLUME : 0.0f);
        break;
    default:
        break;
    }
}

void Game::openDialogue(const std::string& id)
{
    if (m_dialogue.start(id)) {
        changeState(GameState::Dialogue);
    }
}

// ------------------------------------------------------------------ atualizacao

void Game::handleMovement(float deltaTime, bool allowMove)
{
    // Movimento relativo a camera: W anda para onde a camera olha
    Vec3 forward = forwardFromYaw(m_camera.getYaw());
    Vec3 right = { -forward.z, 0.0f, forward.x };
    Vec3 move = { 0.0f, 0.0f, 0.0f };

    if (allowMove) {
        if (Input::isKeyDown(GLFW_KEY_W) || Input::isKeyDown(GLFW_KEY_UP))    move = add(move, forward);
        if (Input::isKeyDown(GLFW_KEY_S) || Input::isKeyDown(GLFW_KEY_DOWN))  move = subtract(move, forward);
        if (Input::isKeyDown(GLFW_KEY_D) || Input::isKeyDown(GLFW_KEY_RIGHT)) move = add(move, right);
        if (Input::isKeyDown(GLFW_KEY_A) || Input::isKeyDown(GLFW_KEY_LEFT))  move = subtract(move, right);
    }

    bool running = allowMove && (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT) || Input::isKeyDown(GLFW_KEY_RIGHT_SHIFT));

    m_player.update(deltaTime, normalize(move), running, *m_scene);

    // Sons de passos conforme o piso
    if (m_player.tookStep()) {
        Vec3 position = m_player.getPosition();
        int variation = (m_stepCounter++) % 4;
        Surface surface = m_scene->surfaceAt(position.x, position.z);
        const Sound& step = surface == Surface::Wood ? m_stepsWood[variation]
                          : surface == Surface::Stone ? m_stepsStone[variation]
                          : m_stepsSand[variation];
        m_audio.play(step, running ? 0.45f : 0.32f, false);
    }
}

void Game::updateWorld(float deltaTime)
{
    Vec3 position = m_player.getPosition();
    m_scene->update(deltaTime, m_time, position);

    // Ondas mais altas perto do mar
    float closeness = clamp(1.0f - m_scene->distanceToSea(position) / 30.0f, 0.0f, 1.0f);
    m_audio.setVolume(m_wavesVoice, 0.12f + 0.5f * closeness);

    const Scene& scene = *m_scene;
    m_camera.setAspect(m_window.getAspect());
    m_camera.update(position, deltaTime, [&scene](float x, float z) { return scene.groundHeight(x, z); });
}

void Game::update(float deltaTime)
{
    // No modo de captura automatica o teclado e o mouse sao ignorados
    if (std::getenv("ROTA_AUTOSHOT") == nullptr) {
        Input::update();
    }

    if (Input::wasKeyPressed(GLFW_KEY_M)) {
        m_musicOn = !m_musicOn;
        float volume = m_state == GameState::Paused ? MUSIC_VOLUME * 0.35f : MUSIC_VOLUME;
        m_audio.setVolume(m_musicVoice, m_musicOn ? volume : 0.0f);
    }

    bool pauseKey = Input::wasKeyPressed(GLFW_KEY_ESCAPE) || Input::wasKeyPressed(GLFW_KEY_P);

    // PAUSA: o mundo para; so da para continuar ou sair do jogo
    if (m_state == GameState::Paused) {
        if (pauseKey) {
            changeState(m_stateBeforePause);
        }
        else if (Input::wasKeyPressed(GLFW_KEY_Q)) {
            m_window.close();
        }
        return;
    }

    if (pauseKey && m_state != GameState::Transition) {
        m_stateBeforePause = m_state;
        changeState(GameState::Paused);
        return;
    }

    m_time += deltaTime;
    handleCamera();

    bool closePressed = Input::wasKeyPressed(GLFW_KEY_E) || Input::wasKeyPressed(GLFW_KEY_SPACE) ||
                        (Input::wasMouseReleased(GLFW_MOUSE_BUTTON_LEFT) && m_dragDistance < 6.0f);

    switch (m_state) {
    case GameState::Playing:
        handleBoatControls(deltaTime);
        handleMovement(deltaTime, true);
        updateWorld(deltaTime);
        handleInteraction();

        if (Input::wasKeyPressed(GLFW_KEY_F2)) {
            changeState(GameState::Transition);
        }
        else if (Input::wasKeyPressed(GLFW_KEY_F3)) {
            changeState(GameState::Challenge);
        }
        break;

    case GameState::Dialogue:
        // A Lia fica parada, mas a agua, os coqueiros e o barco continuam animando
        handleMovement(deltaTime, false);
        updateWorld(deltaTime);
        m_dialogue.update(deltaTime);

        // Clique/E/Espaco: mostra a fala inteira, passa para a proxima ou fecha na ultima
        if (closePressed) {
            m_audio.play(m_blip, 0.35f, false);
            if (!m_dialogue.advance()) {
                changeState(GameState::Playing);
            }
        }
        break;

    case GameState::Challenge:
        handleMovement(deltaTime, false);
        updateWorld(deltaTime);

        if (closePressed) {
            changeState(GameState::Playing);
        }
        break;

    case GameState::Transition:
        handleMovement(deltaTime, false);
        updateWorld(deltaTime);

        m_transitionTimer += deltaTime;
        if (m_transitionTimer >= TRANSITION_SECONDS) {
            changeState(GameState::Playing);
        }
        break;

    case GameState::Paused:
        break;
    }
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

    renderInterface();
}

void Game::renderInterface()
{
    int width = m_window.getWidth();
    int height = m_window.getHeight();
    if (width <= 0 || height <= 0) {
        return;
    }

    // Tamanho do texto acompanha a altura da janela (referencia: 720 px)
    float ui = height / 720.0f;

    m_text.begin(m_renderer, width, height);

    // Dica de controles, discreta, no canto superior esquerdo
    float hintScale = 0.5f * ui;
    float hintWidth = m_text.measure(HINT, hintScale);
    m_text.drawBox(10.0f * ui, 10.0f * ui, hintWidth + 16.0f * ui, m_text.lineHeight(hintScale) + 8.0f * ui, { 0.0f, 0.0f, 0.0f, 0.25f });
    m_text.drawText(18.0f * ui, 14.0f * ui, HINT, hintScale, { 1.0f, 1.0f, 1.0f, 0.8f });

    // Caixa de dialogo / desafio na parte de baixo da tela
    bool inDialogue = m_state == GameState::Dialogue && m_dialogue.isActive();
    bool inChallenge = m_state == GameState::Challenge && !m_message.empty();

    if (inDialogue || inChallenge) {
        float scale = 0.8f * ui;
        float padding = 18.0f * ui;
        float boxWidth = width * 0.7f;
        float textWidth = boxWidth - 2.0f * padding;
        float promptScale = 0.5f * ui;

        // Altura fixa de 3 linhas para a caixa nao "pular" enquanto as letras aparecem
        float boxHeight = 3.0f * m_text.lineHeight(scale) + m_text.lineHeight(promptScale) + 2.0f * padding;
        float boxX = (width - boxWidth) * 0.5f;
        float boxY = height - boxHeight - 30.0f * ui;

        Color accent = inChallenge ? Color{ 0.35f, 0.75f, 0.95f, 0.95f } : Color{ 0.98f, 0.76f, 0.24f, 0.95f };

        m_text.drawBox(boxX, boxY, boxWidth, boxHeight, { 0.05f, 0.08f, 0.12f, 0.78f });
        m_text.drawBox(boxX, boxY, boxWidth, 3.0f * ui, accent);

        // Nome de quem fala, numa etiqueta em destaque acima da caixa
        std::string speaker = inDialogue ? m_dialogue.speaker() : std::string("Desafio");
        if (!speaker.empty()) {
            float nameScale = 0.7f * ui;
            float nameWidth = m_text.measure(speaker, nameScale) + 2.0f * padding;
            float nameHeight = m_text.lineHeight(nameScale) + 8.0f * ui;
            m_text.drawBox(boxX, boxY - nameHeight, nameWidth, nameHeight, accent);
            m_text.drawText(boxX + padding, boxY - nameHeight + 4.0f * ui, speaker, nameScale, { 0.08f, 0.08f, 0.1f, 1.0f });
        }

        std::string text = inDialogue ? m_dialogue.visibleText() : m_message;
        Color textColor = inDialogue && m_dialogue.speaker().empty() ? Color{ 0.85f, 0.92f, 1.0f, 1.0f } : Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        m_text.drawWrapped(boxX + padding, boxY + padding, textWidth, text, scale, textColor);

        bool ready = !inDialogue || m_dialogue.isLineComplete();
        if (ready) {
            const char* prompt = (inDialogue && !m_dialogue.isLastLine()) ? "E, Espaço ou clique: continuar" : "E, Espaço ou clique: fechar";
            float promptWidth = m_text.measure(prompt, promptScale);
            m_text.drawText(boxX + boxWidth - padding - promptWidth, boxY + boxHeight - padding - m_text.lineHeight(promptScale),
                            prompt, promptScale, { 1.0f, 1.0f, 1.0f, 0.6f });
        }
    }

    // TRANSICAO: escurece ate o preto na primeira metade e clareia na segunda
    if (m_state == GameState::Transition) {
        float half = TRANSITION_SECONDS * 0.5f;
        float alpha = m_transitionTimer < half ? m_transitionTimer / half : 1.0f - (m_transitionTimer - half) / half;
        m_text.drawBox(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), { 0.0f, 0.0f, 0.0f, clamp(alpha, 0.0f, 1.0f) });
    }

    // PAUSA: escurece a cena parada e mostra os controles
    if (m_state == GameState::Paused) {
        m_text.drawBox(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), { 0.0f, 0.0f, 0.0f, 0.55f });

        float titleScale = 1.6f * ui;
        float lineScale = 0.62f * ui;
        float panelWidth = 760.0f * ui;
        float panelX = (width - panelWidth) * 0.5f;
        float y = height * 0.18f;

        const char* title = "Pausa";
        m_text.drawText((width - m_text.measure(title, titleScale)) * 0.5f, y, title, titleScale, { 1.0f, 0.85f, 0.4f, 1.0f });
        y += m_text.lineHeight(titleScale) + 16.0f * ui;

        int count = static_cast<int>(sizeof(PAUSE_LINES) / sizeof(PAUSE_LINES[0]));
        float panelHeight = count * m_text.lineHeight(lineScale) + 32.0f * ui;
        m_text.drawBox(panelX, y, panelWidth, panelHeight, { 0.05f, 0.08f, 0.12f, 0.8f });
        y += 16.0f * ui;

        for (int i = 0; i < count; ++i) {
            m_text.drawText(panelX + 24.0f * ui, y, PAUSE_LINES[i], lineScale, { 1.0f, 1.0f, 1.0f, 0.95f });
            y += m_text.lineHeight(lineScale);
        }

        y += 32.0f * ui;
        const char* options = "Esc ou P: continuar    ·    Q: sair do jogo";
        float optionsScale = 0.8f * ui;
        m_text.drawText((width - m_text.measure(options, optionsScale)) * 0.5f, y, options, optionsScale, { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    m_text.end();
}
