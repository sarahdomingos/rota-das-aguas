#include "Boat.h"

#include "core/AssetPath.h"
#include "graphics/Renderer.h"
#include "graphics/Water.h"

#include <cmath>
#include <iostream>

using namespace MathUtils;

namespace {

// Altura da origem do barco acima da agua: deixa a borda sempre acima das ondas
const float WATERLINE_OFFSET = 0.1f;

}

Boat::Boat()
    : m_baseX(0.0f),
      m_baseZ(0.0f),
      m_highlight(0.0f)
{
    // Casco simples, usado so se o .obj nao carregar
    m_fallbackHull.addSphere({ 0.0f, 0.0f, 0.0f }, { 0.32f, 0.22f, 1.0f }, 10, 6, { 0.6f, 0.42f, 0.28f });

    // O modelo e normalizado para ~2 unidades; dobramos para ficar maior que a Lia.
    m_transform.scale(2.0f, 2.0f, 2.0f);
}

bool Boat::load()
{
    m_wood.loadBMP(AssetPath::resolve("assets/models/textura_madeira.bmp"));

    if (m_model.load(AssetPath::resolve("assets/models/barco_mundau.obj"))) {
        return true;
    }

    std::cerr << "Usando o casco simples no lugar do modelo do barco." << std::endl;
    return false;
}

void Boat::setMooring(float x, float z, float yawDegrees)
{
    m_baseX = x;
    m_baseZ = z;
    m_transform.positionX = x;
    m_transform.positionZ = z;
    m_transform.rotationY = yawDegrees;
}

void Boat::update(float time)
{
    float x = m_transform.positionX;
    float z = m_transform.positionZ;

    // Acompanha a superficie da agua: altura e inclinacao no centro do barco
    float dx = 0.0f;
    float dz = 0.0f;
    Water::slope(x, z, time, dx, dz);

    m_transform.positionY = Water::height(x, z, time) + WATERLINE_OFFSET;
    m_transform.rotationX = -std::atan(dz) * 180.0f / PI * 0.8f;
    m_transform.rotationZ = std::atan(dx) * 180.0f / PI * 0.8f;
}

void Boat::draw(Renderer& renderer, float) const
{
    float matrix[16];
    getMatrix(matrix);

    Material material;
    material.texture = &m_wood;
    material.specular = 0.15f;
    material.shininess = 24.0f;
    material.highlight = m_highlight;

    if (m_model.isLoaded()) {
        // A cor do .mtl e cinza-claro; deixa a textura mostrar a madeira
        material.color = { 1.2f, 1.2f, 1.2f };
        renderer.draw(m_model, matrix, material);
    }
    else {
        renderer.draw(m_fallbackHull, matrix, material);
    }
}

void Boat::getMatrix(float matrix[16]) const
{
    m_transform.getMatrix(matrix);
}

Vec3 Boat::getPosition() const
{
    return { m_transform.positionX, m_transform.positionY, m_transform.positionZ };
}

int Boat::getColliders(float out[][3], int maximum) const
{
    float matrix[16];
    getMatrix(matrix);

    const float offsets[3] = { -0.6f, 0.0f, 0.6f };
    int count = 0;

    for (int i = 0; i < 3 && count < maximum; ++i) {
        Vec3 p = transformPoint(matrix, { 0.0f, 0.0f, offsets[i] });
        out[count][0] = p.x;
        out[count][1] = p.z;
        out[count][2] = 0.36f * m_transform.scaleX;
        ++count;
    }

    return count;
}

void Boat::setHighlight(float value)
{
    m_highlight = value;
}
