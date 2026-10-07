#ifndef ENTITY_H
#define ENTITY_H

// Classe base dos objetos do mundo: todo objeto tem uma Transform
// (posicao, rotacao e escala) e sabe se desenhar.

#include "graphics/Transform.h"

class Renderer;

class Entity {
public:
    virtual ~Entity() {}

    virtual void draw(Renderer& renderer, float time) const = 0;

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

protected:
    Transform m_transform;
};

#endif
