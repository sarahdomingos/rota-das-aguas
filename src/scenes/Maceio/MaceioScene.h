#ifndef MACEIO_SCENE_H
#define MACEIO_SCENE_H

// Capitulo 1 - Maceio: orla com areia, coqueiros, pier de madeira, calcadao,
// casario colorido, pedras, guarda-sois e o barco Mundau atracado.
// Ambiente ficticio inspirado na cidade (sem precisao geografica).

#include "scenes/Scene.h"
#include "entities/Boat.h"
#include "graphics/Mesh.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"

#include <string>
#include <vector>

class MaceioScene : public Scene {
public:
    MaceioScene();

    bool init() override;
    void update(float deltaTime, float time, const Vec3& playerPosition) override;

    void drawSky(Renderer& renderer, const Camera& camera, float time) override;
    void drawOpaque(Renderer& renderer, float time) override;
    void drawShadows(Renderer& renderer, const Vec3& playerPosition) override;
    void drawWater(Renderer& renderer) override;

    const Light& getLight() const override;
    Vec3 getPlayerStart() const override;

    float groundHeight(float x, float z) const override;
    Surface surfaceAt(float x, float z) const override;
    void resolveMovement(Vec3& position, const Vec3& previous, float radius) const override;
    float distanceToSea(const Vec3& position) const override;

    int findInteractable(const Vec3& playerPosition, const Vec3* rayOrigin, const Vec3* rayDirection) const override;
    void setFocus(int id) override;
    Interaction interact(int id) override;

    Boat& getBoat();

private:
    struct Circle { float x, z, radius; };
    struct Box { float minX, minZ, maxX, maxZ; };

    struct Palm {
        Mesh trunk;
        float matrix[16];
        Vec3 top;       // topo do tronco, em coordenadas do mundo
        Vec3 localTop;
        float phase;
        float shake;
    };

    struct Placed {
        const Mesh* mesh;
        float matrix[16];
    };

    enum class Kind { Boat, Sign, Palm };

    struct Interactable {
        Kind kind;
        int index;
        Vec3 position;
        float range;
        float pickRadius;
        std::string message;
        InteractionSound sound;
        float flash;
    };

    Light m_light;

    Texture m_sand;
    Texture m_wood;
    Texture m_water;
    Texture m_plaster;
    Texture m_stone;

    Mesh m_terrain;
    Mesh m_pavement;
    Mesh m_walls;
    Mesh m_roofs;
    Mesh m_pier;
    Mesh m_rocks;
    Mesh m_lamps;
    Mesh m_umbrellas;
    Mesh m_sign;
    Mesh m_frond;
    Mesh m_waterGrid;
    Mesh m_skyDome;
    Mesh m_sun;
    Mesh m_disc;
    std::vector<Mesh> m_cloudMeshes;
    std::vector<Placed> m_clouds;

    std::vector<Palm> m_palms;
    std::vector<Vec3> m_umbrellaSpots;
    float m_signMatrix[16];
    Vec3 m_signPosition;

    Boat m_boat;

    std::vector<Circle> m_circles;
    std::vector<Box> m_boxes;
    std::vector<Interactable> m_interactables;
    int m_focus;
    float m_time;

    float terrainHeight(float x, float z) const;
    float terrainVisualHeight(float x, float z) const;
    bool onPier(float x, float z) const;
    bool isWalkable(float x, float z) const;
    float highlightFor(Kind kind, int index) const;

    void buildTerrain();
    void buildTown();
    void buildBeach();
    void buildSky();
    void drawBlobShadow(Renderer& renderer, const Vec3& point, float radius, float opacity);
};

#endif
