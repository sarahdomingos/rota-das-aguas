#include "Boat.h"

#include "core/AssetPath.h"
#include "core/Input.h"
#include "graphics/Renderer.h"

#include <cmath>
#include <iostream>

Boat::Boat()
    : m_fallbackHull(Mesh::createBoatHull()),
      m_baseY(0.0f),
      m_time(0.0f)
{
    // O modelo e normalizado para ~2 unidades; dobramos para ficar maior que a Lia.
    m_transform.scale(2.0f, 2.0f, 2.0f);
}

bool Boat::load()
{
    if (m_model.load(AssetPath::resolve("assets/models/barco_mundau.obj"))) {
        return true;
    }

    std::cerr << "Usando o casco simples no lugar do modelo do barco." << std::endl;
    return false;
}

void Boat::setPosition(float x, float y, float z)
{
    m_transform.positionX = x;
    m_transform.positionY = y;
    m_transform.positionZ = z;
    m_baseY = y;
}

void Boat::update(float deltaTime)
{
    m_time += deltaTime;

    // Balanco na agua: translacao vertical e uma leve rotacao no eixo Z
    m_transform.positionY = m_baseY + 0.08f * std::sin(m_time * 1.6f);
    m_transform.rotationZ = 3.0f * std::sin(m_time * 1.1f);

    // Controles da etapa anterior: Q / E giram e Z / X mudam a escala do barco
    const float rotationSpeed = 60.0f * deltaTime;
    const float scaleFactor = 1.0f + 0.5f * deltaTime;

    if (Input::isKeyDown(GLFW_KEY_Q)) m_transform.rotate(0.0f, rotationSpeed, 0.0f);
    if (Input::isKeyDown(GLFW_KEY_E)) m_transform.rotate(0.0f, -rotationSpeed, 0.0f);

    if (Input::isKeyDown(GLFW_KEY_Z) && m_transform.scaleX > 0.5f) {
        m_transform.scale(1.0f / scaleFactor, 1.0f / scaleFactor, 1.0f / scaleFactor);
    }

    if (Input::isKeyDown(GLFW_KEY_X) && m_transform.scaleX < 6.0f) {
        m_transform.scale(scaleFactor, scaleFactor, scaleFactor);
    }
}

void Boat::draw(Renderer& renderer) const
{
    float matrix[16];
    m_transform.getMatrix(matrix);

    const Vec3 wood = { 0.75f, 0.50f, 0.30f };

    if (m_model.isLoaded()) {
        renderer.draw(m_model, matrix, wood);
    }
    else {
        renderer.draw(m_fallbackHull, matrix, wood);
    }
}
