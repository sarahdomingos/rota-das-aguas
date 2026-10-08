#include "NPC.h"

#include "graphics/Renderer.h"

#include <cmath>

using namespace MathUtils;

namespace {

const float NOTICE_DISTANCE = 4.5f; // a partir de onde o NPC vira para a Lia
const float TURN_SPEED = 3.0f;

}

NPC::NPC(const std::string& name, const CharacterStyle& style, const std::string& dialogueId)
    : m_name(name),
      m_dialogueId(dialogueId),
      m_model(LiaModel::build(style)),
      m_height(style.height),
      m_baseYaw(0.0f),
      m_idlePhase(static_cast<float>(name.size()) * 1.7f),
      m_highlight(0.0f)
{
    m_transform.scaleX = m_height;
    m_transform.scaleY = m_height;
    m_transform.scaleZ = m_height;
}

void NPC::place(const Vec3& position, float yawDegrees)
{
    m_transform.positionX = position.x;
    m_transform.positionY = position.y;
    m_transform.positionZ = position.z;
    m_transform.rotationY = yawDegrees;
    m_baseYaw = yawDegrees;
}

void NPC::update(float deltaTime, const Vec3& playerPosition)
{
    float dx = playerPosition.x - m_transform.positionX;
    float dz = playerPosition.z - m_transform.positionZ;
    float distance = std::sqrt(dx * dx + dz * dz);

    // Olha para a Lia quando ela chega perto; depois volta a posicao original
    float targetYaw = m_baseYaw;
    if (distance < NOTICE_DISTANCE && distance > 0.01f) {
        targetYaw = std::atan2(-dx, -dz) * 180.0f / PI;
    }

    float blend = 1.0f - std::exp(-TURN_SPEED * deltaTime);
    m_transform.rotationY += angleDifference(m_transform.rotationY, targetYaw) * blend;
}

void NPC::draw(Renderer& renderer, float time) const
{
    float root[16];
    m_transform.getMatrix(root);

    // Parado: so a respiracao e o balanco leve dos bracos (fase propria de cada NPC)
    LiaModel::draw(m_model, renderer, root, 0.0f, 0.0f, time + m_idlePhase, m_highlight);
}

Vec3 NPC::getPosition() const
{
    return { m_transform.positionX, m_transform.positionY, m_transform.positionZ };
}

float NPC::getRadius() const
{
    return 0.3f * m_height;
}

const std::string& NPC::getName() const
{
    return m_name;
}

const std::string& NPC::getDialogueId() const
{
    return m_dialogueId;
}

void NPC::setHighlight(float value)
{
    m_highlight = value;
}
