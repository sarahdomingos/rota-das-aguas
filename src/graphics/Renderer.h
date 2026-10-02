#ifndef RENDERER_H
#define RENDERER_H

#include "Model.h"
#include "Transform.h"

class Renderer {
public:
    Renderer();

    bool loadBoat();

    void render();

    Transform& getBoatTransform();

private:
    Model m_boat;
    Transform m_boatTransform;
};

#endif