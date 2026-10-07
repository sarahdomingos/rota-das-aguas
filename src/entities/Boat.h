#ifndef BOAT_H
#define BOAT_H

// Embarcacao Mundau, atracada no pier. Usa o modelo
// assets/models/barco_mundau.obj com a textura de madeira, e acompanha as
// ondas (sobe, desce e inclina conforme a superficie da agua).

#include "Entity.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include "graphics/Texture.h"

class Boat : public Entity {
public:
    Boat();

    bool load();

    void setMooring(float x, float z, float yawDegrees);

    void update(float time);
    void draw(Renderer& renderer, float time) const override;

    void getMatrix(float matrix[16]) const;
    Vec3 getPosition() const;

    // Circulos de colisao ao longo do casco (centro x, z e raio).
    int getColliders(float out[][3], int maximum) const;

    // Brilho de destaque (objeto em foco/interagido), definido pela cena.
    void setHighlight(float value);

private:
    Model m_model;
    Mesh m_fallbackHull;
    Texture m_wood;

    float m_baseX;
    float m_baseZ;
    float m_highlight;
};

#endif
