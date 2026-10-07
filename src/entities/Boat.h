#ifndef BOAT_H
#define BOAT_H

// Embarcacao Mundau. Usa o modelo assets/models/barco_mundau.obj e,
// se ele nao carregar, um casco simples gerado por codigo.

#include "Entity.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"

class Boat : public Entity {
public:
    Boat();

    bool load();

    void setPosition(float x, float y, float z);

    void update(float deltaTime) override;
    void draw(Renderer& renderer) const override;

private:
    Model m_model;
    Mesh m_fallbackHull;

    float m_baseY;
    float m_time;
};

#endif
