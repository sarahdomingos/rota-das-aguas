#include "Renderer.h"

#include <GL/gl.h>

Renderer::Renderer()
{
}

bool Renderer::loadBoat()
{
    return m_boat.load(
        "assets/models/barco_mundau.obj"
    );
}

void Renderer::render()
{
    if (!m_boat.isLoaded()) {
        return;
    }

    float matrix[16];

    m_boatTransform.getMatrix(matrix);

    glPushMatrix();

    glMultMatrixf(matrix);

    m_boat.draw();

    glPopMatrix();
}

Transform& Renderer::getBoatTransform()
{
    return m_boatTransform;
}