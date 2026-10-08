#include "Player.h"

#include "graphics/Renderer.h"
#include "scenes/Scene.h"

#include <cmath>

using namespace MathUtils;

namespace {

const float WALK_SPEED = 3.2f;
const float RUN_SPEED = 6.0f;
const float TURN_SPEED = 600.0f;
const float RADIUS = 0.28f;

}

Player::Player()
    : m_walkPhase(0.0f),
      m_walkAmount(0.0f),
      m_stepped(false)
{
}

void Player::build()
{
    m_model = LiaModel::build();
}

void Player::placeAt(const Vec3& position, float yawDegrees)
{
    m_transform.positionX = position.x;
    m_transform.positionY = position.y;
    m_transform.positionZ = position.z;
    m_transform.rotationY = yawDegrees;
}

void Player::update(float deltaTime, const Vec3& moveDirection, bool running, const Scene& scene)
{
    m_stepped = false;

    float amount = length(moveDirection);
    float speed = running ? RUN_SPEED : WALK_SPEED;

    Vec3 previous = getPosition();
    Vec3 position = previous;

    if (amount > 0.001f) {
        Vec3 direction = scale(moveDirection, 1.0f / amount);

        // Rotacao: gira aos poucos ate ficar de frente para onde anda
        float targetYaw = std::atan2(-direction.x, -direction.z) * 180.0f / PI;
        float difference = angleDifference(m_transform.rotationY, targetYaw);
        float maxTurn = TURN_SPEED * deltaTime;
        m_transform.rotationY += clamp(difference, -maxTurn, maxTurn);

        // Translacao
        position = add(position, scale(direction, speed * deltaTime));
    }

    scene.resolveMovement(position, previous, RADIUS);

    // Acompanha o chao suavemente (degraus do pier, calcadao)
    float ground = scene.groundHeight(position.x, position.z);
    float follow = 1.0f - std::exp(-18.0f * deltaTime);
    position.y = lerp(previous.y, ground, follow);

    m_transform.positionX = position.x;
    m_transform.positionY = position.y;
    m_transform.positionZ = position.z;

    // Animacao de caminhada proporcional a distancia realmente percorrida
    float travelled = length(subtract({ position.x, 0.0f, position.z }, { previous.x, 0.0f, previous.z }));
    float targetAmount = (amount > 0.001f && travelled > 0.0001f) ? (running ? 1.4f : 1.0f) : 0.0f;
    m_walkAmount = lerp(m_walkAmount, targetAmount, 1.0f - std::exp(-10.0f * deltaTime));

    float before = std::floor(m_walkPhase / PI);
    m_walkPhase += travelled * 2.6f;
    if (std::floor(m_walkPhase / PI) != before && m_walkAmount > 0.3f) {
        m_stepped = true;
    }
}

void Player::draw(Renderer& renderer, float time) const
{
    float root[16];
    m_transform.getMatrix(root);
    LiaModel::draw(m_model, renderer, root, m_walkPhase, m_walkAmount, time);
}

Vec3 Player::getPosition() const
{
    return { m_transform.positionX, m_transform.positionY, m_transform.positionZ };
}

float Player::getYaw() const
{
    return m_transform.rotationY;
}

float Player::getRadius() const
{
    return RADIUS;
}

bool Player::tookStep() const
{
    return m_stepped;
}
