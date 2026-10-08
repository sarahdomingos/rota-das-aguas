#include "Item.h"

#include "graphics/Renderer.h"

#include <cmath>

Item::Item(const std::string& itemId, int quantity, const Mesh* mesh, float x, float groundY, float z, float phase)
    : m_itemId(itemId),
      m_quantity(quantity),
      m_mesh(mesh),
      m_groundY(groundY),
      m_phase(phase),
      m_collected(false),
      m_highlight(0.0f)
{
    m_transform.positionX = x;
    m_transform.positionY = groundY;
    m_transform.positionZ = z;
}

void Item::draw(Renderer& renderer, float time) const
{
    if (m_collected || m_mesh == nullptr) {
        return;
    }

    // Flutua e gira devagar para chamar a atencao
    Transform transform = m_transform;
    transform.positionY = m_groundY + 0.15f + 0.07f * std::sin(time * 2.2f + m_phase);
    transform.rotationY = time * 60.0f + m_phase * 40.0f;
    transform.scaleX = transform.scaleY = transform.scaleZ = 1.4f;

    float matrix[16];
    transform.getMatrix(matrix);

    Material material;
    material.specular = 0.3f;
    material.shininess = 30.0f;
    material.twoSided = true;
    material.highlight = m_highlight + 0.06f + 0.04f * std::sin(time * 3.0f + m_phase);
    renderer.draw(*m_mesh, matrix, material);
}

const std::string& Item::getItemId() const
{
    return m_itemId;
}

int Item::getQuantity() const
{
    return m_quantity;
}

Vec3 Item::getPosition() const
{
    return { m_transform.positionX, m_groundY, m_transform.positionZ };
}

bool Item::isCollected() const
{
    return m_collected;
}

void Item::collect()
{
    m_collected = true;
}

void Item::setHighlight(float value)
{
    m_highlight = value;
}
