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
const char* HINT = "WASD: andar  ·  Shift: correr  ·  arrastar o mouse: câmera  ·  roda: zoom  ·  clique ou E: interagir  ·  I: mochila  ·  Tab: missão  ·  M: música  ·  Esc/P: pausa";

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
    "E perto do barco: embarcar  ·  no barco, WASD pilota e E desembarca (perto do píer ou da areia)",
    "R / T: girar o barco  ·  Z / X: tamanho do barco (com a Lia fora dele)",
    "I: mochila (inventário)  ·  Tab: quadro da missão  ·  M: música",
    "F12: captura de tela",
    "F2: teste de transição"
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
      m_transitionTimer(0.0f),
      m_toastTimer(0.0f),
      m_inventorySelection(0),
      m_answerValue(0),
      m_inBoat(false),
      m_boatSpeed(0.0f)
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
    m_pickup = SoundSynth::pickup();
    m_success = SoundSynth::success();
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
    m_quests.loadFile(AssetPath::resolve("assets/missoes/maceio.txt"));
    m_challenges.loadFile(AssetPath::resolve("assets/desafios/maceio.txt"));
    openDialogue("inicio");

    // Para testes: ROTA_TEST_STATE=pausa abre o jogo ja na tela de pausa
    const char* testState = std::getenv("ROTA_TEST_STATE");
    if (testState != nullptr && std::string(testState) == "desafio") {
        std::map<std::string, int> variables;
        variables["entregue"] = 3;
        variables["tinha"] = 3;
        startChallenge("cocos_graca", variables);
    }
    if (testState != nullptr && std::string(testState) == "missao") {
        m_quests.start("cocos_graca");
        m_inventory.add("coco", 1);
        changeState(GameState::Playing);
    }
    if (testState != nullptr && std::string(testState) == "barco") {
        changeState(GameState::Playing);
        boardBoat();
    }
    if (testState != nullptr && std::string(testState) == "quadro") {
        m_quests.start("cocos_graca");
        m_inventory.add("coco", 2);
        openBoard("cocos_graca");
    }
    if (testState != nullptr && std::string(testState) == "entrega") {
        m_quests.start("cocos_graca");
        m_answerQuest = "cocos_graca";
        m_answerValue = 4;
        changeState(GameState::Answer);
    }
    if (testState != nullptr && std::string(testState) == "mochila") {
        m_inventory.add("coco", 2);
        m_inventory.add("concha", 1);
        changeState(GameState::Inventory);
    }
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
        if (m_inBoat && (focused < 0 || m_maceio.isBoat(focused))) {
            tryDisembark();
            return;
        }
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

    if (m_inBoat && m_maceio.isBoat(target)) {
        return;
    }

    Interaction result = m_scene->interact(target);

    // Barco: a Lia embarca
    if (result.boat) {
        boardBoat();
        return;
    }

    // Item: pega, toca o som e mostra um aviso rapido
    if (!result.itemId.empty()) {
        m_audio.play(m_pickup, 0.5f, false);
        onItemCollected(result.itemId, result.itemQuantity);
        return;
    }

    m_audio.play(result.sound == InteractionSound::Leaves ? m_leaves : m_chime, 0.55f, false);

    if (result.sound == InteractionSound::Leaves) {
        m_audio.play(m_chime, 0.25f, false);
    }

    const QuestDefinition* quest = m_quests.openQuestFor(result.dialogueId);

    if (quest != nullptr && m_quests.status(quest->id) == QuestStatus::Available) {
        m_pendingQuestStart = quest->id;
        openDialogue(quest->dialogueStart.empty() ? result.dialogueId : quest->dialogueStart);
    }
    else if (quest != nullptr) {
        m_answerQuest = quest->id;
        m_answerValue = 0;
        changeState(GameState::Answer);
    }
    else {
        openDialogue(result.dialogueId);
    }
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

// ------------------------------------------------------------------ barco

void Game::boardBoat()
{
    m_inBoat = true;
    m_boatSpeed = 0.0f;
    m_maceio.setBoatOccupied(true);
    m_player.stopWalking();
    m_audio.play(m_stepsWood[0], 0.4f, false);
    showToast("A Lia entrou no Mundaú! WASD: pilotar  ·  E: desembarcar perto do píer ou da areia");
}

void Game::tryDisembark()
{
    Vec3 landing;
    if (!m_maceio.findLanding(m_maceio.getBoat().getPosition(), landing)) {
        showToast("Chegue perto do píer ou da areia para desembarcar.");
        return;
    }

    m_inBoat = false;
    m_boatSpeed = 0.0f;
    m_maceio.setBoatOccupied(false);
    m_player.placeAt(landing, m_player.getYaw());
    m_audio.play(m_stepsWood[1], 0.4f, false);
}

void Game::handleSailing(float deltaTime)
{
    Transform& boat = m_maceio.getBoat().getTransform();

    if (deltaTime > 0.0f) {
        // W/S aceleram para frente/tras, A/D viram (mais devagar parado)
        float throttle = 0.0f;
        float turn = 0.0f;
        if (Input::isKeyDown(GLFW_KEY_W) || Input::isKeyDown(GLFW_KEY_UP))    throttle += 1.0f;
        if (Input::isKeyDown(GLFW_KEY_S) || Input::isKeyDown(GLFW_KEY_DOWN))  throttle -= 0.5f;
        if (Input::isKeyDown(GLFW_KEY_A) || Input::isKeyDown(GLFW_KEY_LEFT))  turn += 1.0f;
        if (Input::isKeyDown(GLFW_KEY_D) || Input::isKeyDown(GLFW_KEY_RIGHT)) turn -= 1.0f;

        const float maxSpeed = 5.0f;
        m_boatSpeed += throttle * 4.0f * deltaTime;
        m_boatSpeed -= m_boatSpeed * 0.9f * deltaTime; // arrasto da agua
        m_boatSpeed = clamp(m_boatSpeed, -maxSpeed * 0.5f, maxSpeed);

        float turnRate = 55.0f * (0.35f + 0.65f * std::fmin(std::fabs(m_boatSpeed) / maxSpeed, 1.0f));
        boat.rotationY += turn * turnRate * deltaTime;

        // Anda so se o centro e as duas pontas continuarem em agua navegavel
        Vec3 forward = forwardFromYaw(boat.rotationY);
        float nextX = boat.positionX + forward.x * m_boatSpeed * deltaTime;
        float nextZ = boat.positionZ + forward.z * m_boatSpeed * deltaTime;
        float half = 1.9f * boat.scaleX * 0.5f;

        bool free = m_maceio.isNavigable(nextX, nextZ) &&
                    m_maceio.isNavigable(nextX + forward.x * half, nextZ + forward.z * half) &&
                    m_maceio.isNavigable(nextX - forward.x * half, nextZ - forward.z * half);

        if (free) {
            boat.positionX = nextX;
            boat.positionZ = nextZ;
        }
        else {
            m_boatSpeed = 0.0f;
        }

        // A camera vai virando para ficar atras do barco enquanto ele anda
        if (std::fabs(m_boatSpeed) > 0.3f) {
            float follow = std::fmin(1.0f, deltaTime * 1.2f);
            m_camera.orbit(angleDifference(m_camera.getYaw(), boat.rotationY) * follow, 0.0f);
        }
    }

    // A Lia vai sentada no barco
    float matrix[16];
    m_maceio.getBoat().getMatrix(matrix);
    Vec3 seat = transformPoint(matrix, { 0.0f, -0.12f, 0.25f });
    m_player.placeAt(seat, boat.rotationY);
    m_player.stopWalking();
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

void Game::showToast(const std::string& text)
{
    m_toast = text;
    m_toastTimer = 2.5f;
}

void Game::onItemCollected(const std::string& itemId, int quantity)
{
    m_inventory.add(itemId, quantity);

    char buffer[160];
    std::snprintf(buffer, sizeof(buffer), "Você pegou: %s (+%d)", m_scene->getItemCatalog().nameOf(itemId).c_str(), quantity);
    showToast(buffer);
}

void Game::onDialogueFinished()
{
    // Fim do pedido do NPC: a missao comeca e o quadro abre sozinho
    if (!m_pendingQuestStart.empty()) {
        std::string id = m_pendingQuestStart;
        m_pendingQuestStart.clear();
        m_quests.start(id);

        const QuestDefinition* quest = m_quests.find(id);
        showToast("Nova missão: " + (quest != nullptr ? quest->title : id));
        m_audio.play(m_chime, 0.4f, false);
        openBoard(id);
        return;
    }

    // Fim da fala de conclusao: mostra a conta montada
    if (!m_pendingChallengeQuest.empty()) {
        const QuestDefinition* quest = m_quests.find(m_pendingChallengeQuest);
        m_pendingChallengeQuest.clear();

        if (quest != nullptr && !quest->challenge.empty()) {
            startChallenge(quest->challenge, std::map<std::string, int>());
        }
    }
}

int Game::countFor(const std::string& itemKey) const
{
    // A missao pode pedir um item (ex.: "coco") ou uma categoria inteira (ex.: "lixo")
    const ItemCatalog& catalog = m_scene->getItemCatalog();
    int total = 0;

    for (const std::pair<std::string, int>& entry : m_inventory.entries()) {
        const ItemDefinition* definition = catalog.find(entry.first);
        if (entry.first == itemKey || (definition != nullptr && definition->category == itemKey)) {
            total += entry.second;
        }
    }
    return total;
}

bool Game::removeFor(const std::string& itemKey, int quantity)
{
    if (countFor(itemKey) < quantity) {
        return false;
    }

    const ItemCatalog& catalog = m_scene->getItemCatalog();
    for (const std::pair<std::string, int>& entry : m_inventory.entries()) {
        if (quantity <= 0) break;

        const ItemDefinition* definition = catalog.find(entry.first);
        if (entry.first == itemKey || (definition != nullptr && definition->category == itemKey)) {
            int amount = entry.second < quantity ? entry.second : quantity;
            m_inventory.remove(entry.first, amount);
            quantity -= amount;
        }
    }
    return true;
}

std::string Game::itemNameFor(const std::string& itemKey) const
{
    const ItemDefinition* definition = m_scene->getItemCatalog().find(itemKey);
    if (definition != nullptr) {
        return definition->name;
    }

    // Categoria: primeira letra maiuscula (ex.: "lixo" -> "Lixo")
    std::string name = itemKey;
    if (!name.empty() && name[0] >= 'a' && name[0] <= 'z') {
        name[0] = static_cast<char>(name[0] - 'a' + 'A');
    }
    return name;
}

int Game::answerFor(const QuestDefinition& quest)
{
    // O resultado da conta do desafio e a quantidade certa a entregar
    MathChallenge challenge;
    if (!m_challenges.build(quest.challenge, std::map<std::string, int>(), challenge) || !challenge.valid) {
        return -1;
    }
    return challenge.result;
}

void Game::openBoard(const std::string& questId)
{
    m_boardQuest = questId;
    changeState(GameState::Board);
}

void Game::confirmAnswer()
{
    const QuestDefinition* quest = m_quests.find(m_answerQuest);
    if (quest == nullptr) {
        changeState(GameState::Playing);
        return;
    }

    m_quests.addAttempt(quest->id);

    // So conclui com a resposta certa E exatamente essa quantidade na mochila
    int answer = answerFor(*quest);
    bool correct = answer >= 0 && m_answerValue == answer && countFor(quest->itemKey) == answer;

    changeState(GameState::Playing);

    if (correct) {
        removeFor(quest->itemKey, answer);
        m_quests.complete(quest->id);
        showToast("Missão concluída: " + quest->title);
        m_pendingChallengeQuest = quest->id;

        if (quest->dialogueComplete.empty() || !m_dialogue.has(quest->dialogueComplete)) {
            onDialogueFinished();
        }
        else {
            openDialogue(quest->dialogueComplete);
        }
    }
    else {
        // Errou: so o aviso do NPC, sem dica; a Lia precisa falar com ele de novo
        openDialogue(quest->dialogueWrong);
    }
}

void Game::startChallenge(const std::string& id, const std::map<std::string, int>& variables)
{
    if (!m_challenges.build(id, variables, m_challenge)) {
        return;
    }

    m_message = m_challenge.context;
    changeState(GameState::Challenge);
    m_audio.play(m_success, 0.55f, false);
}

void Game::updateInventoryScreen()
{
    std::vector<std::pair<std::string, int>> items = m_inventory.entries();
    if (items.empty()) {
        return;
    }

    int count = static_cast<int>(items.size());
    if (Input::wasKeyPressed(GLFW_KEY_DOWN) || Input::wasKeyPressed(GLFW_KEY_S)) m_inventorySelection++;
    if (Input::wasKeyPressed(GLFW_KEY_UP) || Input::wasKeyPressed(GLFW_KEY_W)) m_inventorySelection--;
    m_inventorySelection = (m_inventorySelection % count + count) % count;

    // G solta 1 unidade do item selecionado
    if (Input::wasKeyPressed(GLFW_KEY_G)) {
        dropFromInventory(items[m_inventorySelection].first);
        return;
    }

    // Clique numa linha solta 1 unidade daquele item
    if (Input::wasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        int windowWidth = 1, windowHeight = 1;
        glfwGetWindowSize(m_window.getNativeWindow(), &windowWidth, &windowHeight);
        float mx = static_cast<float>(Input::mouseX() * m_window.getWidth() / windowWidth);
        float my = static_cast<float>(Input::mouseY() * m_window.getHeight() / windowHeight);

        for (size_t i = 0; i < m_inventoryRows.size(); ++i) {
            const InventoryRow& row = m_inventoryRows[i];
            if (mx >= row.x0 && mx <= row.x1 && my >= row.y0 && my <= row.y1) {
                m_inventorySelection = static_cast<int>(i);
                dropFromInventory(row.itemId);
                return;
            }
        }
    }
}

void Game::dropFromInventory(const std::string& itemId)
{
    if (!m_inventory.remove(itemId, 1)) {
        return;
    }

    m_scene->dropItem(itemId, m_player.getPosition(), m_player.getYaw());
    m_audio.play(m_blip, 0.4f, false);
    showToast("Você soltou: " + m_scene->getItemCatalog().nameOf(itemId) + " (-1)");
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

    // MOCHILA: o mundo para enquanto a tela do inventario esta aberta
    if (m_state == GameState::Inventory) {
        if (Input::wasKeyPressed(GLFW_KEY_I) || Input::wasKeyPressed(GLFW_KEY_ESCAPE)) {
            changeState(GameState::Playing);
        }
        else {
            updateInventoryScreen();
        }
        return;
    }

    // QUADRO DA MISSAO: fecha com Tab, Esc, E, Enter ou Espaco
    if (m_state == GameState::Board) {
        if (Input::wasKeyPressed(GLFW_KEY_TAB) || Input::wasKeyPressed(GLFW_KEY_ESCAPE) || Input::wasKeyPressed(GLFW_KEY_E) ||
            Input::wasKeyPressed(GLFW_KEY_ENTER) || Input::wasKeyPressed(GLFW_KEY_SPACE)) {
            changeState(GameState::Playing);
        }
        return;
    }

    // ENTREGA: setas mudam o numero, Enter/E confirmam, Esc volta sem contar tentativa
    if (m_state == GameState::Answer) {
        if (Input::wasKeyPressed(GLFW_KEY_RIGHT) || Input::wasKeyPressed(GLFW_KEY_UP) ||
            Input::wasKeyPressed(GLFW_KEY_D) || Input::wasKeyPressed(GLFW_KEY_W)) {
            m_answerValue = m_answerValue < 999 ? m_answerValue + 1 : 999;
            m_audio.play(m_blip, 0.25f, false);
        }
        if (Input::wasKeyPressed(GLFW_KEY_LEFT) || Input::wasKeyPressed(GLFW_KEY_DOWN) ||
            Input::wasKeyPressed(GLFW_KEY_A) || Input::wasKeyPressed(GLFW_KEY_S)) {
            m_answerValue = m_answerValue > 0 ? m_answerValue - 1 : 0;
            m_audio.play(m_blip, 0.25f, false);
        }
        if (Input::wasKeyPressed(GLFW_KEY_ENTER) || Input::wasKeyPressed(GLFW_KEY_E)) {
            confirmAnswer();
        }
        else if (Input::wasKeyPressed(GLFW_KEY_ESCAPE)) {
            changeState(GameState::Playing);
        }
        return;
    }

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
    if (m_toastTimer > 0.0f) {
        m_toastTimer -= deltaTime;
    }
    handleCamera();

    bool closePressed = Input::wasKeyPressed(GLFW_KEY_E) || Input::wasKeyPressed(GLFW_KEY_SPACE) ||
                        (Input::wasMouseReleased(GLFW_MOUSE_BUTTON_LEFT) && m_dragDistance < 6.0f);

    switch (m_state) {
    case GameState::Playing:
        if (m_inBoat) {
            handleSailing(deltaTime);
        }
        else {
            handleBoatControls(deltaTime);
            handleMovement(deltaTime, true);
        }
        updateWorld(deltaTime);
        handleInteraction();

        if (Input::wasKeyPressed(GLFW_KEY_I)) {
            changeState(GameState::Inventory);
        }
        else if (Input::wasKeyPressed(GLFW_KEY_TAB)) {
            const QuestDefinition* quest = m_quests.activeQuest();
            if (quest != nullptr) {
                openBoard(quest->id);
            }
            else {
                showToast("Nenhuma missão em andamento.");
            }
        }
        else if (Input::wasKeyPressed(GLFW_KEY_F2)) {
            changeState(GameState::Transition);
        }
        break;

    case GameState::Dialogue:
        // A Lia fica parada, mas a agua, os coqueiros e o barco continuam animando
        if (m_inBoat) {
            handleSailing(0.0f);
        }
        else {
            handleMovement(deltaTime, false);
        }
        updateWorld(deltaTime);
        m_dialogue.update(deltaTime);

        // Clique/E/Espaco: mostra a fala inteira, passa para a proxima ou fecha na ultima
        if (closePressed) {
            m_audio.play(m_blip, 0.35f, false);
            if (!m_dialogue.advance()) {
                changeState(GameState::Playing);
                onDialogueFinished();
            }
        }
        break;

    case GameState::Challenge:
        if (m_inBoat) {
            handleSailing(0.0f);
        }
        else {
            handleMovement(deltaTime, false);
        }
        updateWorld(deltaTime);

        if (closePressed) {
            changeState(GameState::Playing);
        }
        break;

    case GameState::Transition:
        if (m_inBoat) {
            handleSailing(0.0f);
        }
        else {
            handleMovement(deltaTime, false);
        }
        updateWorld(deltaTime);

        m_transitionTimer += deltaTime;
        if (m_transitionTimer >= TRANSITION_SECONDS) {
            changeState(GameState::Playing);
        }
        break;

    case GameState::Paused:
    case GameState::Inventory:
    case GameState::Board:
    case GameState::Answer:
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

    if (m_state == GameState::Playing || m_state == GameState::Dialogue) {
        renderHud(ui, width);
    }

    // Aviso rapido no alto da tela (ex.: item pego)
    if (m_toastTimer > 0.0f && !m_toast.empty()) {
        float fade = m_toastTimer < 0.5f ? m_toastTimer / 0.5f : 1.0f;
        float toastScale = 0.75f * ui;
        float toastWidth = m_text.measure(m_toast, toastScale) + 40.0f * ui;
        float toastHeight = m_text.lineHeight(toastScale) + 16.0f * ui;
        float toastX = (width - toastWidth) * 0.5f;
        float toastY = 70.0f * ui;
        m_text.drawBox(toastX, toastY, toastWidth, toastHeight, { 0.05f, 0.08f, 0.12f, 0.7f * fade });
        m_text.drawText(toastX + 20.0f * ui, toastY + 8.0f * ui, m_toast, toastScale, { 1.0f, 0.9f, 0.5f, fade });
    }

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
        std::string speaker = inDialogue ? m_dialogue.speaker() : std::string("Desafio resolvido!");
        if (!speaker.empty()) {
            float nameScale = 0.7f * ui;
            float nameWidth = m_text.measure(speaker, nameScale) + 2.0f * padding;
            float nameHeight = m_text.lineHeight(nameScale) + 8.0f * ui;
            m_text.drawBox(boxX, boxY - nameHeight, nameWidth, nameHeight, accent);
            m_text.drawText(boxX + padding, boxY - nameHeight + 4.0f * ui, speaker, nameScale, { 0.08f, 0.08f, 0.1f, 1.0f });
        }

        if (inChallenge) {
            // A conta montada, em destaque acima da caixa
            std::string expression = m_challenge.text();
            float bigScale = 1.8f * ui;
            float panelWidth = m_text.measure(expression, bigScale) + 80.0f * ui;
            float panelHeight = m_text.lineHeight(bigScale) + 30.0f * ui;
            float panelX = (width - panelWidth) * 0.5f;
            float panelY = boxY - panelHeight - 70.0f * ui;
            m_text.drawBox(panelX, panelY, panelWidth, panelHeight, { 0.05f, 0.08f, 0.12f, 0.85f });
            m_text.drawBox(panelX, panelY + panelHeight - 4.0f * ui, panelWidth, 4.0f * ui, accent);
            m_text.drawText(panelX + 40.0f * ui, panelY + 15.0f * ui, expression, bigScale, { 1.0f, 1.0f, 1.0f, 1.0f });
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

    if (m_state == GameState::Inventory) {
        renderInventoryScreen(ui, width, height);
    }

    if (m_state == GameState::Board) {
        renderBoard(ui, width, height);
    }

    if (m_state == GameState::Answer) {
        renderAnswer(ui, width, height);
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

void Game::renderHud(float ui, int width)
{
    // Missao atual, no canto superior esquerdo (abaixo da dica de controles)
    if (const QuestDefinition* quest = m_quests.activeQuest()) {
        char progress[96];
        std::snprintf(progress, sizeof(progress), "%s: %d   ·   Tab: quadro da missão", itemNameFor(quest->itemKey).c_str(),
                      countFor(quest->itemKey));

        float titleScale = 0.6f * ui;
        float lineScale = 0.5f * ui;
        float boxWidth = 380.0f * ui;
        float x = 10.0f * ui;
        float y = 46.0f * ui;

        std::vector<std::string> lines = m_text.wrap("Ajude " + quest->giverName + " a " + quest->task + ".", lineScale, boxWidth - 24.0f * ui);
        float boxHeight = m_text.lineHeight(titleScale) + (lines.size() + 1) * m_text.lineHeight(lineScale) + 16.0f * ui;

        m_text.drawBox(x, y, boxWidth, boxHeight, { 0.0f, 0.0f, 0.0f, 0.35f });
        m_text.drawBox(x, y, 4.0f * ui, boxHeight, { 0.98f, 0.76f, 0.24f, 0.9f });
        m_text.drawText(x + 12.0f * ui, y + 6.0f * ui, "Missão: " + quest->title, titleScale, { 1.0f, 0.85f, 0.4f, 1.0f });

        float lineY = y + 6.0f * ui + m_text.lineHeight(titleScale);
        for (const std::string& line : lines) {
            m_text.drawText(x + 12.0f * ui, lineY, line, lineScale, { 1.0f, 1.0f, 1.0f, 0.9f });
            lineY += m_text.lineHeight(lineScale);
        }
        m_text.drawText(x + 12.0f * ui, lineY, progress, lineScale, { 0.75f, 0.95f, 0.75f, 1.0f });
    }

    // Itens que a Lia carrega, no canto superior direito
    std::vector<std::pair<std::string, int>> items = m_inventory.entries();
    if (items.empty()) {
        return;
    }

    const ItemCatalog& catalog = m_scene->getItemCatalog();
    float scale = 0.55f * ui;
    float row = m_text.lineHeight(scale) + 4.0f * ui;
    float panelWidth = 210.0f * ui;
    float x = width - panelWidth - 10.0f * ui;
    float y = 10.0f * ui;

    m_text.drawBox(x, y, panelWidth, row * items.size() + 12.0f * ui, { 0.0f, 0.0f, 0.0f, 0.3f });

    for (size_t i = 0; i < items.size(); ++i) {
        const ItemDefinition* definition = catalog.find(items[i].first);
        Vec3 color = definition != nullptr ? definition->color : Vec3{ 1.0f, 1.0f, 1.0f };
        float rowY = y + 6.0f * ui + row * i;

        m_text.drawBox(x + 10.0f * ui, rowY + 5.0f * ui, 14.0f * ui, 14.0f * ui, { color.x, color.y, color.z, 1.0f });

        char line[128];
        std::snprintf(line, sizeof(line), "%s  ×%d", catalog.nameOf(items[i].first).c_str(), items[i].second);
        m_text.drawText(x + 32.0f * ui, rowY, line, scale, { 1.0f, 1.0f, 1.0f, 0.95f });
    }
}

void Game::renderInventoryScreen(float ui, int width, int height)
{
    m_text.drawBox(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), { 0.0f, 0.0f, 0.0f, 0.5f });

    const ItemCatalog& catalog = m_scene->getItemCatalog();
    std::vector<std::pair<std::string, int>> items = m_inventory.entries();

    float titleScale = 1.2f * ui;
    float lineScale = 0.7f * ui;
    float row = m_text.lineHeight(lineScale) + 8.0f * ui;
    float panelWidth = 620.0f * ui;
    float panelHeight = (items.empty() ? 1 : items.size()) * row + 40.0f * ui;
    float panelX = (width - panelWidth) * 0.5f;
    float y = height * 0.2f;

    const char* title = "Mochila da Lia";
    m_text.drawText((width - m_text.measure(title, titleScale)) * 0.5f, y, title, titleScale, { 1.0f, 0.85f, 0.4f, 1.0f });
    y += m_text.lineHeight(titleScale) + 14.0f * ui;

    m_text.drawBox(panelX, y, panelWidth, panelHeight, { 0.05f, 0.08f, 0.12f, 0.85f });
    y += 20.0f * ui;

    if (items.empty()) {
        m_text.drawText(panelX + 24.0f * ui, y, "A mochila está vazia. Procure itens pela orla!", lineScale, { 1.0f, 1.0f, 1.0f, 0.8f });
    }

    m_inventoryRows.clear();
    if (!items.empty()) {
        int count = static_cast<int>(items.size());
        m_inventorySelection = (m_inventorySelection % count + count) % count;
    }

    for (size_t i = 0; i < items.size(); ++i) {
        const ItemDefinition* definition = catalog.find(items[i].first);
        Vec3 color = definition != nullptr ? definition->color : Vec3{ 1.0f, 1.0f, 1.0f };
        std::string category = definition != nullptr ? definition->category : "";

        m_inventoryRows.push_back({ panelX + 8.0f * ui, y - 2.0f * ui, panelX + panelWidth - 8.0f * ui, y + row - 6.0f * ui, items[i].first });
        if (static_cast<int>(i) == m_inventorySelection) {
            m_text.drawBox(panelX + 8.0f * ui, y - 2.0f * ui, panelWidth - 16.0f * ui, row - 4.0f * ui, { 0.98f, 0.76f, 0.24f, 0.25f });
        }

        m_text.drawBox(panelX + 24.0f * ui, y + 6.0f * ui, 20.0f * ui, 20.0f * ui, { color.x, color.y, color.z, 1.0f });
        m_text.drawText(panelX + 56.0f * ui, y, catalog.nameOf(items[i].first), lineScale, { 1.0f, 1.0f, 1.0f, 1.0f });
        m_text.drawText(panelX + 300.0f * ui, y, category, lineScale * 0.85f, { 0.75f, 0.85f, 1.0f, 0.8f });

        char quantity[32];
        std::snprintf(quantity, sizeof(quantity), "×%d", items[i].second);
        m_text.drawText(panelX + panelWidth - 24.0f * ui - m_text.measure(quantity, lineScale), y, quantity, lineScale, { 1.0f, 0.9f, 0.5f, 1.0f });
        y += row;
    }

    y = height * 0.2f + m_text.lineHeight(titleScale) + 14.0f * ui + panelHeight + 24.0f * ui;
    const char* hint = "Setas: escolher  ·  G ou clique: soltar 1  ·  I ou Esc: fechar a mochila";
    m_text.drawText((width - m_text.measure(hint, 0.6f * ui)) * 0.5f, y, hint, 0.6f * ui, { 1.0f, 1.0f, 1.0f, 0.8f });
}

void Game::renderBoard(float ui, int width, int height)
{
    const QuestDefinition* quest = m_quests.find(m_boardQuest);
    if (quest == nullptr) {
        return;
    }

    m_text.drawBox(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), { 0.0f, 0.0f, 0.0f, 0.6f });

    const Color yellow = { 1.0f, 0.85f, 0.4f, 1.0f };
    const Color white = { 1.0f, 1.0f, 1.0f, 0.95f };
    const Color panel = { 0.05f, 0.08f, 0.12f, 0.88f };

    float margin = width * 0.06f;
    float y = height * 0.07f;

    // Topo: titulo e o pedido de ajuda
    m_text.drawText(margin, y, "MISSÃO: " + quest->title, 1.2f * ui, yellow);
    y += m_text.lineHeight(1.2f * ui) + 4.0f * ui;

    std::string help = quest->giverName + " precisa da sua ajuda! Ajude-" + quest->pronoun + " a " + quest->task + ".";
    y += m_text.drawWrapped(margin, y, width - 2.0f * margin, help, 0.7f * ui, white) + 14.0f * ui;

    float panelTop = y;
    float panelBottom = height - 60.0f * ui;
    float leftWidth = (width - 2.0f * margin) * 0.38f;
    float rightX = margin + leftWidth + 20.0f * ui;
    float rightWidth = width - margin - rightX;
    float pad = 16.0f * ui;

    m_text.drawBox(margin, panelTop, leftWidth, panelBottom - panelTop, panel);
    m_text.drawBox(rightX, panelTop, rightWidth, panelBottom - panelTop, panel);

    // Painel esquerdo: dados do problema e passo a passo (sem o resultado)
    float lineScale = 0.55f * ui;
    float textWidth = leftWidth - 2.0f * pad;
    float ly = panelTop + pad;

    m_text.drawText(margin + pad, ly, "Dados do problema", 0.62f * ui, yellow);
    ly += m_text.lineHeight(0.62f * ui) + 4.0f * ui;
    for (const std::string& fact : quest->facts) {
        ly += m_text.drawWrapped(margin + pad, ly, textWidth, "• " + fact, lineScale, white) + 4.0f * ui;
    }

    ly += 12.0f * ui;
    m_text.drawText(margin + pad, ly, "Passo a passo", 0.62f * ui, yellow);
    ly += m_text.lineHeight(0.62f * ui) + 4.0f * ui;
    for (size_t i = 0; i < quest->steps.size(); ++i) {
        char number[8];
        std::snprintf(number, sizeof(number), "%d. ", static_cast<int>(i + 1));
        ly += m_text.drawWrapped(margin + pad, ly, textWidth, number + quest->steps[i], lineScale, white) + 4.0f * ui;
    }

    // Painel direito: os itens em 3D, agrupados em fileiras, e o que esta na mochila
    struct Icon { const Mesh* mesh; float x; float y; float size; };
    std::vector<Icon> icons;

    float ry = panelTop + pad;
    float cell = 42.0f * ui;

    for (const QuestIllustration& group : quest->illustrations) {
        m_text.drawText(rightX + pad, ry, group.label, 0.6f * ui, yellow);
        ry += m_text.lineHeight(0.6f * ui) + 4.0f * ui;

        const Mesh* mesh = m_scene->getItemMesh(group.itemId);
        int rows = (group.count + group.perRow - 1) / group.perRow;
        float size = cell * 0.9f;
        if (group.perRow * cell > rightWidth - 2.0f * pad) {
            size = (rightWidth - 2.0f * pad) / group.perRow * 0.9f;
        }
        float step = size / 0.9f;

        for (int i = 0; i < group.count; ++i) {
            int row = i / group.perRow;
            int column = i % group.perRow;
            icons.push_back({ mesh, rightX + pad + (column + 0.5f) * step, ry + (row + 0.5f) * step * 0.85f, size });
        }

        ry += rows * step * 0.85f + 14.0f * ui;
    }

    char carried[96];
    std::snprintf(carried, sizeof(carried), "Na mochila: %d", countFor(quest->itemKey));
    m_text.drawText(rightX + pad, panelBottom - pad - m_text.lineHeight(0.65f * ui), carried, 0.65f * ui, { 0.75f, 0.95f, 0.75f, 1.0f });

    const char* hint = "Tab, Esc ou E: fechar o quadro   ·   para entregar, fale com quem pediu";
    m_text.drawText((width - m_text.measure(hint, 0.5f * ui)) * 0.5f, height - 40.0f * ui, hint, 0.5f * ui, { 1.0f, 1.0f, 1.0f, 0.75f });

    // Os modelos 3D por ultimo, girando, por cima dos paineis
    m_renderer.beginIcons(width, height);
    for (size_t i = 0; i < icons.size(); ++i) {
        if (icons[i].mesh != nullptr) {
            m_renderer.drawIcon(*icons[i].mesh, icons[i].x, icons[i].y, icons[i].size, m_time * 50.0f + i * 23.0f);
        }
    }
    m_renderer.endIcons(width, height);
}

void Game::renderAnswer(float ui, int width, int height)
{
    const QuestDefinition* quest = m_quests.find(m_answerQuest);
    if (quest == nullptr) {
        return;
    }

    float scale = 0.75f * ui;
    float padding = 18.0f * ui;
    float boxWidth = width * 0.7f;
    float boxHeight = 220.0f * ui;
    float boxX = (width - boxWidth) * 0.5f;
    float boxY = height - boxHeight - 30.0f * ui;
    Color accent = { 0.98f, 0.76f, 0.24f, 0.95f };

    m_text.drawBox(boxX, boxY, boxWidth, boxHeight, { 0.05f, 0.08f, 0.12f, 0.85f });
    m_text.drawBox(boxX, boxY, boxWidth, 3.0f * ui, accent);

    // Etiqueta com o nome de quem pediu
    float nameScale = 0.7f * ui;
    float nameWidth = m_text.measure(quest->giverName, nameScale) + 2.0f * padding;
    float nameHeight = m_text.lineHeight(nameScale) + 8.0f * ui;
    m_text.drawBox(boxX, boxY - nameHeight, nameWidth, nameHeight, accent);
    m_text.drawText(boxX + padding, boxY - nameHeight + 4.0f * ui, quest->giverName, nameScale, { 0.08f, 0.08f, 0.1f, 1.0f });

    std::string question = "Qual é o resultado da conta? Essa é a quantidade de " + itemNameFor(quest->itemKey) +
                           " que você vai me entregar.";
    m_text.drawWrapped(boxX + padding, boxY + padding, boxWidth - 2.0f * padding, question, scale, { 1.0f, 1.0f, 1.0f, 1.0f });

    // Seletor do numero
    char number[32];
    std::snprintf(number, sizeof(number), "<   %d   >", m_answerValue);
    float bigScale = 1.6f * ui;
    m_text.drawText((width - m_text.measure(number, bigScale)) * 0.5f, boxY + 80.0f * ui, number, bigScale, { 1.0f, 0.9f, 0.5f, 1.0f });

    const char* hint = "Setas: mudar o número  ·  Enter ou E: confirmar  ·  Esc: voltar";
    float hintScale = 0.5f * ui;
    m_text.drawText((width - m_text.measure(hint, hintScale)) * 0.5f, boxY + boxHeight - padding - m_text.lineHeight(hintScale),
                    hint, hintScale, { 1.0f, 1.0f, 1.0f, 0.65f });
}
