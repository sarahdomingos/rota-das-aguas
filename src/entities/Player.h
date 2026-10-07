#ifndef PLAYER_H
#define PLAYER_H

// Lia, a personagem principal. Por enquanto representada por cubos
// (corpo, cabeca e um "nariz" que mostra para onde ela esta virada).

#include "Entity.h"
#include "graphics/MathUtils.h"

class Mesh;

class Player : public Entity {
public:
    explicit Player(const Mesh& cube);

    void update(float deltaTime) override;
    void draw(Renderer& renderer) const override;

    Vec3 getPosition() const;
    float getYaw() const;

    void setGroundHeight(float height);

private:
    const Mesh& m_cube;

    float m_walkSpeed;
    float m_runSpeed;
    float m_turnSpeed;
    float m_worldLimit;

    void drawPart(Renderer& renderer, const float parent[16], const Transform& local, const Vec3& color) const;
};

#endif
