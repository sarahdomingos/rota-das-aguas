#ifndef ENTITY_H
#define ENTITY_H

// Classe base/interface dos objetos do mundo: todo objeto tem uma Transform
// (posicao, rotacao e escala) e sabe se atualizar e se desenhar.

#include "graphics/Transform.h"

class Renderer;

class Entity {
public:
    virtual ~Entity() {}

    virtual void update(float deltaTime) = 0;
    virtual void draw(Renderer& renderer) const = 0;

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

protected:
    Transform m_transform;
};

#endif
