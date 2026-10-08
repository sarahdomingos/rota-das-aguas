#include "MaceioScene.h"

#include "graphics/Camera.h"
#include "graphics/ProceduralTextures.h"
#include "models/CoastalModels.h"
#include "models/UrbanModels.h"

#include <cmath>
#include <cstdint>

using namespace MathUtils;

namespace {

// Layout da orla (em z): mar < 0 < areia < calcadao < rua < calcada < casas
const float PROMENADE_START = 6.5f;
const float STREET_START = 10.5f;
const float SIDEWALK_START = 14.0f;
const float LOTS_START = 16.0f;
const float PROMENADE_TOP = 0.62f;
const float STREET_TOP = 0.52f;

// Pier (comeca na areia e avanca para o mar, em -Z)
const float PIER_X = 10.0f;
const float PIER_Z = 2.6f;
const float PIER_LENGTH = 25.0f;
const float PIER_WIDTH = 2.6f;
const float PIER_DECK = 0.78f;
const float PIER_GAP_START = -15.9f; // trecho sem guarda-corpo, onde o barco atraca
const float PIER_GAP_END = -21.4f;

const float WORLD_MIN_X = -75.0f;
const float WORLD_MAX_X = 75.0f;
const float WORLD_MIN_Z = -40.0f;
const float WORLD_MAX_Z = 30.0f;

float random01(int a, int b)
{
    uint32_t h = static_cast<uint32_t>(a) * 374761393u + static_cast<uint32_t>(b) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFF) / 65535.0f;
}

void placeMatrix(float x, float y, float z, float yawDegrees, float scaleBy, float out[16])
{
    float local[16];
    translation(x, y, z, out);
    rotationY(yawDegrees, local);
    apply(out, local);
    scaling(scaleBy, scaleBy, scaleBy, local);
    apply(out, local);
}

}

MaceioScene::MaceioScene()
    : m_focus(-1),
      m_time(0.0f)
{
    m_light.sunDirection = { 0.55f, 0.62f, -0.55f };
    m_light.sunColor = { 1.05f, 0.97f, 0.86f };
    m_light.skyAmbient = { 0.42f, 0.50f, 0.62f };
    m_light.groundAmbient = { 0.36f, 0.31f, 0.25f };
    m_light.fogColor = { 0.78f, 0.87f, 0.95f };
    m_light.fogDensity = 0.0085f;

    identity(m_signMatrix);
    m_signPosition = { -2.5f, 0.0f, 2.0f };
}

// ------------------------------------------------------------------ terreno

float MaceioScene::terrainHeight(float x, float z) const
{
    // Calcadao, rua e calcada: pisos planos
    if (z >= PROMENADE_START && z < STREET_START) return PROMENADE_TOP;
    if (z >= STREET_START && z < SIDEWALK_START) return STREET_TOP;
    if (z >= SIDEWALK_START && z < LOTS_START) return PROMENADE_TOP;

    if (z >= LOTS_START) {
        // Gramado atras das casas subindo para morros ao fundo
        float hills = 7.0f * smoothstep(26.0f, 48.0f, z) * (0.6f + 0.4f * std::sin(0.05f * x + 1.0f));
        return 0.55f + hills;
    }

    // Praia: a linha da costa ondula um pouco ao longo de x
    float zz = z + 1.4f * std::sin(0.07f * x) + 0.6f * std::sin(0.19f * x + 1.0f);
    float h = -1.7f + 2.05f * smoothstep(-19.0f, 1.0f, zz);
    h += 0.10f * smoothstep(1.0f, 6.0f, z);

    // Pequenas dunas na areia seca
    float dunes = smoothstep(-2.0f, 1.0f, z) * (1.0f - smoothstep(5.0f, 6.5f, z));
    h += 0.07f * std::sin(0.45f * x + 0.3f) * std::sin(0.6f * z) * dunes;

    return h;
}

float MaceioScene::terrainVisualHeight(float x, float z) const
{
    // Sob o calcadao/rua o terreno fica escondido abaixo das placas de pedra
    if (z >= PROMENADE_START - 0.3f && z < LOTS_START) {
        return 0.45f;
    }
    return terrainHeight(x, z);
}

bool MaceioScene::onPier(float x, float z) const
{
    return std::fabs(x - PIER_X) < PIER_WIDTH * 0.5f && z <= PIER_Z && z >= PIER_Z - PIER_LENGTH;
}

float MaceioScene::groundHeight(float x, float z) const
{
    if (onPier(x, z)) {
        return PIER_DECK;
    }
    return terrainHeight(x, z);
}

Surface MaceioScene::surfaceAt(float x, float z) const
{
    if (onPier(x, z)) return Surface::Wood;
    if (z >= PROMENADE_START && z < LOTS_START) return Surface::Stone;
    return Surface::Sand;
}

float MaceioScene::distanceToSea(const Vec3& position) const
{
    return std::fmax(0.0f, position.z + 5.0f);
}

void MaceioScene::buildTerrain()
{
    auto height = [this](float x, float z) { return terrainVisualHeight(x, z); };

    auto color = [](float x, float y, float z) {
        float noise = 0.94f + 0.08f * random01(static_cast<int>(x * 3.0f), static_cast<int>(z * 3.0f));

        if (z > LOTS_START) {
            Vec3 grass = { 0.40f * noise, 0.58f * noise, 0.27f * noise };
            Vec3 hill = { 0.30f * noise, 0.47f * noise, 0.22f * noise };
            return lerp(grass, hill, smoothstep(0.6f, 6.0f, y));
        }
        if (y < -0.3f) {
            return lerp(Vec3{ 0.66f, 0.60f, 0.46f }, Vec3{ 0.33f, 0.45f, 0.43f }, smoothstep(-0.3f, -1.6f, y));
        }
        if (y < 0.12f) {
            return Vec3{ 0.80f * noise, 0.70f * noise, 0.52f * noise };
        }
        return Vec3{ 0.97f * noise, 0.88f * noise, 0.68f * noise };
    };

    m_terrain = Mesh::createGrid(-100.0f, -45.0f, 100.0f, 50.0f, 134, 64, height, color);

    // Fundo do mar alem do terreno (ate o horizonte)
    const Vec3 deep = { 0.33f, 0.45f, 0.43f };
    m_terrain.addQuad({ -220.0f, -1.7f, -44.9f }, { 220.0f, -1.7f, -44.9f }, { 220.0f, -1.7f, -220.0f }, { -220.0f, -1.7f, -220.0f }, deep);
    m_terrain.addQuad({ -220.0f, -1.7f, 60.0f }, { -99.9f, -1.7f, 60.0f }, { -99.9f, -1.7f, -45.0f }, { -220.0f, -1.7f, -45.0f }, deep);
    m_terrain.addQuad({ 99.9f, -1.7f, 60.0f }, { 220.0f, -1.7f, 60.0f }, { 220.0f, -1.7f, -45.0f }, { 99.9f, -1.7f, -45.0f }, deep);

    // Calcadao (com meio-fio), rua de paralelepipedos e calcada das casas
    m_pavement.addBox({ 0.0f, PROMENADE_TOP - 0.16f, (PROMENADE_START + STREET_START) * 0.5f },
                      { 200.0f, 0.32f, STREET_START - PROMENADE_START }, { 0.93f, 0.90f, 0.85f });
    m_pavement.addBox({ 0.0f, PROMENADE_TOP - 0.15f, PROMENADE_START }, { 200.0f, 0.34f, 0.22f }, { 0.97f, 0.97f, 0.95f });
    m_pavement.addBox({ 0.0f, STREET_TOP - 0.11f, (STREET_START + SIDEWALK_START) * 0.5f },
                      { 200.0f, 0.22f, SIDEWALK_START - STREET_START }, { 0.50f, 0.48f, 0.47f });
    m_pavement.addBox({ 0.0f, PROMENADE_TOP - 0.16f, (SIDEWALK_START + LOTS_START) * 0.5f },
                      { 200.0f, 0.32f, LOTS_START - SIDEWALK_START }, { 0.86f, 0.82f, 0.77f });

    // Faixas onduladas no calcadao (lembram o piso da orla)
    for (int i = -100; i < 100; ++i) {
        float x0 = static_cast<float>(i);
        float x1 = x0 + 1.0f;
        float z0 = 8.5f + 0.5f * std::sin(x0 * 1.2f);
        float z1 = 8.5f + 0.5f * std::sin(x1 * 1.2f);
        float y = PROMENADE_TOP + 0.005f;
        Vec3 dark = { 0.25f, 0.27f, 0.30f };
        m_pavement.addQuad({ x0, y, z0 + 0.25f }, { x1, y, z1 + 0.25f }, { x1, y, z1 - 0.25f }, { x0, y, z0 - 0.25f }, dark);
    }
}

// ------------------------------------------------------------------ cidade

void MaceioScene::buildTown()
{
    const Vec3 walls[7] = {
        { 0.96f, 0.62f, 0.55f }, { 0.98f, 0.84f, 0.45f }, { 0.55f, 0.78f, 0.86f }, { 0.66f, 0.86f, 0.70f },
        { 0.96f, 0.70f, 0.42f }, { 0.80f, 0.66f, 0.86f }, { 0.95f, 0.93f, 0.88f }
    };
    const Vec3 doors[4] = {
        { 0.18f, 0.45f, 0.40f }, { 0.20f, 0.35f, 0.60f }, { 0.45f, 0.28f, 0.18f }, { 0.65f, 0.20f, 0.18f }
    };

    float x = -55.0f;
    int index = 0;

    while (x < 55.0f) {
        UrbanModels::HouseDescription house;
        house.width = 6.0f + 3.0f * random01(index, 1);
        house.depth = 7.0f;
        house.floors = random01(index, 2) < 0.3f ? 2 : 1;
        house.height = house.floors == 2 ? 6.4f : 3.6f + 0.7f * random01(index, 3);
        house.wallColor = walls[index % 7];
        house.trimColor = { 0.97f, 0.96f, 0.92f };
        house.doorColor = doors[(index * 3 + 1) % 4];
        float roofTone = 0.9f + 0.15f * random01(index, 4);
        house.roofColor = { 0.74f * roofTone, 0.36f * roofTone, 0.22f * roofTone };
        house.roof = random01(index, 5) < 0.5f ? UrbanModels::RoofStyle::Parapet : UrbanModels::RoofStyle::Gable;

        float centerX = x + house.width * 0.5f;
        float centerZ = LOTS_START + 0.3f + house.depth * 0.5f;
        float base = 0.55f;

        float matrix[16];
        translation(centerX, base, centerZ, matrix);

        m_walls.append(UrbanModels::houseWalls(house), matrix);
        m_roofs.append(UrbanModels::houseRoof(house), matrix);

        m_boxes.push_back({ centerX - house.width * 0.5f - 0.1f, centerZ - house.depth * 0.5f - 0.15f,
                            centerX + house.width * 0.5f + 0.1f, centerZ + house.depth * 0.5f });

        x += house.width + 0.3f * random01(index, 6);
        ++index;
    }

    // Postes no calcadao
    Mesh lamp = UrbanModels::streetLamp();
    for (float lx = -48.0f; lx <= 48.0f; lx += 12.0f) {
        float matrix[16];
        placeMatrix(lx, PROMENADE_TOP, 7.0f, 90.0f, 1.0f, matrix);
        m_lamps.append(lamp, matrix);
        m_circles.push_back({ lx, 7.0f, 0.2f });
    }

    // Pier de madeira
    float pierMatrix[16];
    translation(PIER_X, 0.0f, PIER_Z, pierMatrix);
    m_pier.append(UrbanModels::pier(PIER_WIDTH, PIER_LENGTH, PIER_DECK, PIER_GAP_START - PIER_Z, PIER_GAP_END - PIER_Z),
                  pierMatrix);

    // Guarda-corpos como paredes finas de colisao (o lado +X tem a abertura do barco)
    float left = PIER_X - PIER_WIDTH * 0.5f - 0.06f;
    float right = PIER_X + PIER_WIDTH * 0.5f + 0.06f;
    float end = PIER_Z - PIER_LENGTH;
    m_boxes.push_back({ left - 0.08f, end, left + 0.08f, PIER_Z });
    m_boxes.push_back({ right - 0.08f, PIER_GAP_START, right + 0.08f, PIER_Z });
    m_boxes.push_back({ right - 0.08f, end, right + 0.08f, PIER_GAP_END });

    // Placa de boas-vindas
    m_sign = UrbanModels::woodenSign();
    m_signPosition.y = terrainHeight(m_signPosition.x, m_signPosition.z) - 0.05f;
    placeMatrix(m_signPosition.x, m_signPosition.y, m_signPosition.z, 0.0f, 1.0f, m_signMatrix);
    m_boxes.push_back({ m_signPosition.x - 0.8f, m_signPosition.z - 0.15f, m_signPosition.x + 0.8f, m_signPosition.z + 0.15f });
}

// ------------------------------------------------------------------ praia

void MaceioScene::buildBeach()
{
    // Coqueiros na areia e no calcadao (x, z)
    const float spots[][2] = {
        { -44.0f, 1.5f }, { -35.0f, 3.2f }, { -26.0f, 0.8f }, { -17.0f, 3.6f }, { -8.0f, 2.0f },
        { 5.0f, 4.6f }, { 17.0f, 2.8f }, { 24.0f, 4.2f }, { 31.0f, 1.2f }, { 40.0f, 3.5f }, { 48.0f, 2.2f },
        { -30.0f, 9.6f }, { -12.0f, 9.6f }, { 18.0f, 9.6f }, { 36.0f, 9.6f }
    };

    m_frond = CoastalModels::palmFrond(2.8f);

    int count = static_cast<int>(sizeof(spots) / sizeof(spots[0]));
    for (int i = 0; i < count; ++i) {
        Palm palm;
        float height = 5.5f + 2.0f * random01(i, 11);
        float lean = 0.8f + 1.0f * random01(i, 12);
        float yaw = 90.0f + 80.0f * (random01(i, 13) - 0.5f); // inclinados para o mar

        palm.trunk = CoastalModels::palmTrunk(height, lean, palm.localTop);

        float x = spots[i][0];
        float z = spots[i][1];
        float y = terrainHeight(x, z) - 0.1f;
        placeMatrix(x, y, z, yaw, 1.0f, palm.matrix);

        palm.top = transformPoint(palm.matrix, palm.localTop);
        palm.phase = random01(i, 14) * 6.28f;
        palm.shake = 0.0f;
        m_palms.push_back(palm);

        m_circles.push_back({ x, z, 0.32f });
    }

    // Pedras na beira da agua
    const float rocks[][3] = {
        { -31.0f, -6.5f, 1.3f }, { -29.0f, -5.0f, 0.8f }, { -33.5f, -4.6f, 0.9f }, { -27.5f, -7.6f, 1.1f },
        { -35.0f, -7.2f, 0.7f }, { 33.0f, -6.0f, 1.0f }, { 35.2f, -4.9f, 0.7f }, { 31.5f, -7.6f, 0.8f }
    };

    for (int i = 0; i < 8; ++i) {
        float x = rocks[i][0];
        float z = rocks[i][1];
        float size = rocks[i][2];
        float matrix[16];
        placeMatrix(x, terrainHeight(x, z) - size * 0.25f, z, random01(i, 21) * 360.0f, 1.0f, matrix);
        m_rocks.append(CoastalModels::rock(i + 3, size), matrix);
        m_circles.push_back({ x, z, size * 0.95f });
    }

    // Guarda-sois
    const float umbrellas[][2] = { { -22.0f, -0.6f }, { -4.5f, 0.2f }, { 21.0f, -1.0f } };
    const Vec3 colorsA[3] = { { 0.90f, 0.22f, 0.20f }, { 0.20f, 0.45f, 0.80f }, { 0.20f, 0.62f, 0.38f } };
    const Vec3 colorsB[3] = { { 0.98f, 0.97f, 0.93f }, { 0.98f, 0.85f, 0.30f }, { 0.98f, 0.97f, 0.93f } };

    for (int i = 0; i < 3; ++i) {
        float x = umbrellas[i][0];
        float z = umbrellas[i][1];
        float y = terrainHeight(x, z);
        float matrix[16];
        placeMatrix(x, y, z, 15.0f * i, 1.0f, matrix);

        float tilt[16];
        rotationZ(8.0f, tilt);
        apply(matrix, tilt);

        m_umbrellas.append(CoastalModels::beachUmbrella(colorsA[i], colorsB[i]), matrix);
        m_umbrellaSpots.push_back({ x, y, z });
        m_circles.push_back({ x, z, 0.18f });
    }
}

void MaceioScene::buildSky()
{
    m_skyDome = Mesh::createSkyDome({ 0.80f, 0.89f, 0.97f }, { 0.28f, 0.53f, 0.86f });
    m_sun.addSphere({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 16, 10, { 1.0f, 0.97f, 0.86f });
    m_disc = Mesh::createDisc(20);

    for (int i = 0; i < 4; ++i) {
        m_cloudMeshes.push_back(CoastalModels::cloud(i + 1));
    }

    for (int i = 0; i < 9; ++i) {
        Placed cloud;
        cloud.mesh = &m_cloudMeshes[i % 4];
        float angle = 2.0f * PI * i / 9.0f + random01(i, 31);
        float distance = 120.0f + 40.0f * random01(i, 32);
        placeMatrix(std::cos(angle) * distance, 45.0f + 20.0f * random01(i, 33), std::sin(angle) * distance - 30.0f,
                    random01(i, 34) * 360.0f, 1.2f + random01(i, 35), cloud.matrix);
        m_clouds.push_back(cloud);
    }
}

bool MaceioScene::init()
{
    const int size = ProceduralTextures::SIZE;
    m_sand.create(size, size, ProceduralTextures::sand());
    m_wood.create(size, size, ProceduralTextures::wood());
    m_water.create(size, size, ProceduralTextures::water());
    m_plaster.create(size, size, ProceduralTextures::plaster());
    m_stone.create(size, size, ProceduralTextures::stone());

    buildTerrain();
    buildTown();
    buildBeach();
    buildSky();

    m_waterGrid = Mesh::createWaterGrid(400.0f, 200);

    m_boat.load();
    m_boat.setMooring(PIER_X + PIER_WIDTH * 0.5f + 1.0f, -18.6f, 0.0f);

    // Objetos com interacao
    m_interactables.push_back({ Kind::Boat, 0, m_boat.getPosition(), 3.2f, 2.2f,
                                "Barco Mundaú: a canoa de madeira que vai levar a Lia pelas águas de Alagoas.",
                                InteractionSound::Chime, 0.0f });
    m_interactables.push_back({ Kind::Sign, 0, add(m_signPosition, { 0.0f, 1.3f, 0.0f }), 2.6f, 1.0f,
                                "Placa: Bem-vinda a Maceió! Siga pela orla até o píer.",
                                InteractionSound::Chime, 0.0f });

    for (size_t i = 0; i < m_palms.size(); ++i) {
        Vec3 base = { m_palms[i].matrix[12], m_palms[i].matrix[13] + 1.2f, m_palms[i].matrix[14] };
        m_interactables.push_back({ Kind::Palm, static_cast<int>(i), base, 2.0f, 0.8f,
                                    "Coqueiro: a Lia balançou o coqueiro!", InteractionSound::Leaves, 0.0f });
    }

    return true;
}

// ------------------------------------------------------------------ atualizacao

void MaceioScene::update(float deltaTime, float time, const Vec3&)
{
    m_time = time;
    m_boat.update(time);
    m_interactables[0].position = add(m_boat.getPosition(), { 0.0f, 0.3f, 0.0f });

    for (Palm& palm : m_palms) {
        palm.shake = std::fmax(0.0f, palm.shake - deltaTime * 0.8f);
    }

    for (Interactable& item : m_interactables) {
        item.flash = std::fmax(0.0f, item.flash - deltaTime * 1.5f);
    }

    m_boat.setHighlight(highlightFor(Kind::Boat, 0));
}

float MaceioScene::highlightFor(Kind kind, int index) const
{
    for (size_t i = 0; i < m_interactables.size(); ++i) {
        const Interactable& item = m_interactables[i];
        if (item.kind != kind || item.index != index) continue;

        float value = item.flash * 0.5f;
        if (static_cast<int>(i) == m_focus) {
            value += 0.10f + 0.06f * std::sin(m_time * 5.0f);
        }
        return value;
    }
    return 0.0f;
}

// ------------------------------------------------------------------ colisao

bool MaceioScene::isWalkable(float x, float z) const
{
    if (onPier(x, z)) return true;
    return terrainHeight(x, z) > -0.22f; // a Lia so molha os pes, nao entra no mar
}

void MaceioScene::resolveMovement(Vec3& position, const Vec3& previous, float radius) const
{
    position.x = clamp(position.x, WORLD_MIN_X, WORLD_MAX_X);
    position.z = clamp(position.z, WORLD_MIN_Z, WORLD_MAX_Z);

    float boatColliders[3][3];
    int boatCount = m_boat.getColliders(boatColliders, 3);

    for (int iteration = 0; iteration < 2; ++iteration) {
        auto pushFromCircle = [&](float cx, float cz, float r) {
            float dx = position.x - cx;
            float dz = position.z - cz;
            float distance = std::sqrt(dx * dx + dz * dz);
            float minimum = r + radius;

            if (distance < minimum) {
                if (distance < 0.0001f) {
                    dx = 1.0f;
                    dz = 0.0f;
                    distance = 1.0f;
                }
                position.x = cx + dx / distance * minimum;
                position.z = cz + dz / distance * minimum;
            }
        };

        for (const Circle& c : m_circles) {
            pushFromCircle(c.x, c.z, c.radius);
        }
        for (int i = 0; i < boatCount; ++i) {
            pushFromCircle(boatColliders[i][0], boatColliders[i][1], boatColliders[i][2]);
        }

        for (const Box& b : m_boxes) {
            float closestX = clamp(position.x, b.minX, b.maxX);
            float closestZ = clamp(position.z, b.minZ, b.maxZ);
            float dx = position.x - closestX;
            float dz = position.z - closestZ;
            float distanceSq = dx * dx + dz * dz;

            if (distanceSq >= radius * radius) continue;

            if (distanceSq > 0.000001f) {
                float distance = std::sqrt(distanceSq);
                position.x = closestX + dx / distance * radius;
                position.z = closestZ + dz / distance * radius;
            }
            else {
                // Centro dentro da caixa: sai pelo lado mais proximo
                float left = position.x - b.minX, right = b.maxX - position.x;
                float back = position.z - b.minZ, front = b.maxZ - position.z;
                float best = std::fmin(std::fmin(left, right), std::fmin(back, front));
                if (best == left) position.x = b.minX - radius;
                else if (best == right) position.x = b.maxX + radius;
                else if (best == back) position.z = b.minZ - radius;
                else position.z = b.maxZ + radius;
            }
        }
    }

    // Agua funda: tenta deslizar por um dos eixos, senao fica onde estava
    if (!isWalkable(position.x, position.z)) {
        if (isWalkable(position.x, previous.z)) {
            position.z = previous.z;
        }
        else if (isWalkable(previous.x, position.z)) {
            position.x = previous.x;
        }
        else {
            position.x = previous.x;
            position.z = previous.z;
        }
    }
}

// ------------------------------------------------------------------ interacao

int MaceioScene::findInteractable(const Vec3& playerPosition, const Vec3* rayOrigin, const Vec3* rayDirection) const
{
    int best = -1;

    if (rayOrigin != nullptr && rayDirection != nullptr) {
        // Clique: o objeto atingido pelo raio do mouse, se estiver perto da Lia
        float bestT = 1e9f;

        for (size_t i = 0; i < m_interactables.size(); ++i) {
            const Interactable& item = m_interactables[i];
            Vec3 toCenter = subtract(item.position, *rayOrigin);
            float t = dot(toCenter, *rayDirection);
            if (t < 0.0f) continue;

            Vec3 closest = add(*rayOrigin, scale(*rayDirection, t));
            if (length(subtract(closest, item.position)) > item.pickRadius) continue;
            if (length(subtract(item.position, playerPosition)) > item.range + 4.0f) continue;

            if (t < bestT) {
                bestT = t;
                best = static_cast<int>(i);
            }
        }

        return best;
    }

    float bestDistance = 1e9f;

    for (size_t i = 0; i < m_interactables.size(); ++i) {
        const Interactable& item = m_interactables[i];
        float dx = item.position.x - playerPosition.x;
        float dz = item.position.z - playerPosition.z;
        float distance = std::sqrt(dx * dx + dz * dz);

        if (distance < item.range && distance < bestDistance) {
            bestDistance = distance;
            best = static_cast<int>(i);
        }
    }

    return best;
}

void MaceioScene::setFocus(int id)
{
    m_focus = id;
}

Interaction MaceioScene::interact(int id)
{
    Interaction result = { "", InteractionSound::Chime };

    if (id < 0 || id >= static_cast<int>(m_interactables.size())) {
        return result;
    }

    Interactable& item = m_interactables[id];
    item.flash = 1.0f;

    if (item.kind == Kind::Palm) {
        m_palms[item.index].shake = 1.0f;
    }

    result.message = item.message;
    result.sound = item.sound;
    return result;
}

Boat& MaceioScene::getBoat()
{
    return m_boat;
}

const Light& MaceioScene::getLight() const
{
    return m_light;
}

Vec3 MaceioScene::getPlayerStart() const
{
    return { 1.0f, terrainHeight(1.0f, 4.0f), 4.0f };
}

// ------------------------------------------------------------------ desenho

void MaceioScene::drawSky(Renderer& renderer, const Camera& camera, float time)
{
    renderer.drawSky(m_skyDome, camera);

    Vec3 eye = camera.getPosition();
    Vec3 sunDir = normalize(m_light.sunDirection);

    glDepthMask(GL_FALSE);

    Material sun;
    sun.unlit = true;
    sun.fog = false;
    float matrix[16];
    placeMatrix(eye.x + sunDir.x * 170.0f, eye.y + sunDir.y * 170.0f, eye.z + sunDir.z * 170.0f, 0.0f, 7.0f, matrix);
    renderer.draw(m_sun, matrix, sun);

    // Nuvens: acompanham a camera (ficam "no infinito") e andam devagar
    Material cloud;
    cloud.fog = false;
    cloud.specular = 0.0f;
    cloud.color = { 1.1f, 1.1f, 1.12f };

    for (const Placed& placed : m_clouds) {
        float m[16];
        float drift[16];
        translation(eye.x + std::fmod(time * 0.8f, 60.0f) - 30.0f, 0.0f, eye.z, drift);
        multiply(drift, placed.matrix, m);
        renderer.draw(*placed.mesh, m, cloud);
    }

    glDepthMask(GL_TRUE);
}

void MaceioScene::drawOpaque(Renderer& renderer, float time)
{
    float identityMatrix[16];
    identity(identityMatrix);

    Material sand;
    sand.texture = &m_sand;
    sand.worldUV = true;
    sand.textureScale = 0.35f;
    sand.specular = 0.04f;
    renderer.draw(m_terrain, identityMatrix, sand);

    Material stone;
    stone.texture = &m_stone;
    stone.textureScale = 0.5f;
    stone.specular = 0.05f;
    renderer.draw(m_pavement, identityMatrix, stone);

    Material plaster;
    plaster.texture = &m_plaster;
    plaster.textureScale = 0.4f;
    plaster.specular = 0.05f;
    renderer.draw(m_walls, identityMatrix, plaster);

    Material roof = plaster;
    roof.textureScale = 1.2f;
    roof.specular = 0.1f;
    renderer.draw(m_roofs, identityMatrix, roof);

    Material rocks = plaster;
    rocks.textureScale = 1.5f;
    rocks.specular = 0.15f;
    rocks.shininess = 30.0f;
    renderer.draw(m_rocks, identityMatrix, rocks);

    Material wood;
    wood.texture = &m_wood;
    wood.textureScale = 0.6f;
    wood.specular = 0.08f;
    renderer.draw(m_pier, identityMatrix, wood);

    Material sign = wood;
    sign.highlight = highlightFor(Kind::Sign, 0);
    renderer.draw(m_sign, m_signMatrix, sign);

    Material metal;
    metal.specular = 0.35f;
    metal.shininess = 40.0f;
    renderer.draw(m_lamps, identityMatrix, metal);

    Material cloth;
    cloth.twoSided = true;
    cloth.specular = 0.05f;
    renderer.draw(m_umbrellas, identityMatrix, cloth);

    // Coqueiros: tronco + folhas balancando ao vento (hierarquia: tronco -> topo -> folha)
    const int fronds = 9;
    for (size_t i = 0; i < m_palms.size(); ++i) {
        const Palm& palm = m_palms[i];

        Material bark = wood;
        bark.textureScale = 1.0f;
        bark.highlight = highlightFor(Kind::Palm, static_cast<int>(i));
        renderer.draw(palm.trunk, palm.matrix, bark);

        Material leaf;
        leaf.twoSided = true;
        leaf.specular = 0.12f;
        leaf.shininess = 18.0f;
        leaf.highlight = bark.highlight;

        float crown[16];
        float local[16];
        for (int k = 0; k < 16; ++k) crown[k] = palm.matrix[k];
        translation(palm.localTop.x, palm.localTop.y, palm.localTop.z, local);
        apply(crown, local);

        for (int f = 0; f < fronds; ++f) {
            float sway = 4.0f * std::sin(time * 1.3f + f * 0.8f + palm.phase)
                       + palm.shake * 16.0f * std::sin(time * 13.0f + f);
            float twist = 3.0f * std::sin(time * 0.9f + f * 1.7f + palm.phase);

            float frond[16];
            for (int k = 0; k < 16; ++k) frond[k] = crown[k];
            rotationY(f * 360.0f / fronds + palm.phase * 10.0f + twist, local);
            apply(frond, local);
            rotationZ(14.0f + 6.0f * ((f % 3) - 1) + sway, local);
            apply(frond, local);

            renderer.draw(m_frond, frond, leaf);
        }
    }

    m_boat.draw(renderer, time);
}

void MaceioScene::drawBlobShadow(Renderer& renderer, const Vec3& point, float radius, float opacity)
{
    // Projeta o ponto no chao na direcao oposta ao sol
    Vec3 sunDir = normalize(m_light.sunDirection);
    float ground = groundHeight(point.x, point.z);
    float heightAbove = std::fmax(point.y - ground, 0.0f);

    float x = point.x - sunDir.x / sunDir.y * heightAbove;
    float z = point.z - sunDir.z / sunDir.y * heightAbove;
    float y = groundHeight(x, z);

    if (y < -0.2f && !onPier(x, z)) {
        return; // nao desenha sombra sobre o mar
    }

    float matrix[16];
    float local[16];
    translation(x, y + 0.03f, z, matrix);
    scaling(radius, 1.0f, radius * 0.85f, local);
    apply(matrix, local);

    Material shadow;
    shadow.unlit = true;
    shadow.color = { 0.0f, 0.0f, 0.0f };
    shadow.alpha = opacity;
    renderer.draw(m_disc, matrix, shadow);
}

void MaceioScene::drawShadows(Renderer& renderer, const Vec3& playerPosition)
{
    drawBlobShadow(renderer, add(playerPosition, { 0.0f, 0.05f, 0.0f }), 0.36f, 0.35f);

    for (const Palm& palm : m_palms) {
        drawBlobShadow(renderer, palm.top, 2.2f, 0.22f);
    }

    for (const Vec3& spot : m_umbrellaSpots) {
        drawBlobShadow(renderer, add(spot, { 0.0f, 2.2f, 0.0f }), 1.3f, 0.25f);
    }
}

void MaceioScene::drawWater(Renderer& renderer)
{
    float boatMatrix[16];
    m_boat.getMatrix(boatMatrix);
    renderer.setBoatMask(boatMatrix);

    float matrix[16];
    translation(0.0f, 0.0f, -60.0f, matrix);

    Material water;
    water.water = true;
    water.color = { 0.10f, 0.42f, 0.55f };
    water.texture = &m_water;
    water.worldUV = true;
    water.textureScale = 0.07f;
    water.specular = 0.7f;
    water.shininess = 220.0f;
    renderer.draw(m_waterGrid, matrix, water);

    renderer.clearBoatMask();
}
