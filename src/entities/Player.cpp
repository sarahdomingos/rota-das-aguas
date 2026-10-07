#include "Player.h"

#include "core/Input.h"
#include "graphics/Mesh.h"
#include "graphics/Renderer.h"

#include <algorithm>

Player::Player(const Mesh& cube)
    : m_cube(cube),
      m_walkSpeed(4.0f),
      m_runSpeed(8.0f),
      m_turnSpeed(150.0f),
      m_worldLimit(40.0f)
{
}

void Player::update(float deltaTime)
{
    // A / D (ou setas) giram a Lia; W / S (ou setas) andam para frente/tras.
    float turn = 0.0f;
    float move = 0.0f;

    if (Input::isKeyDown(GLFW_KEY_A) || Input::isKeyDown(GLFW_KEY_LEFT))  turn += 1.0f;
    if (Input::isKeyDown(GLFW_KEY_D) || Input::isKeyDown(GLFW_KEY_RIGHT)) turn -= 1.0f;
    if (Input::isKeyDown(GLFW_KEY_W) || Input::isKeyDown(GLFW_KEY_UP))    move += 1.0f;
    if (Input::isKeyDown(GLFW_KEY_S) || Input::isKeyDown(GLFW_KEY_DOWN))  move -= 1.0f;

    bool running = Input::isKeyDown(GLFW_KEY_LEFT_SHIFT) || Input::isKeyDown(GLFW_KEY_RIGHT_SHIFT);
    float speed = running ? m_runSpeed : m_walkSpeed;

    // Rotacao: incrementa o angulo em torno do eixo Y
    m_transform.rotate(0.0f, turn * m_turnSpeed * deltaTime, 0.0f);

    // Translacao: anda na direcao para onde a Lia esta virada
    Vec3 forward = MathUtils::forwardFromYaw(m_transform.rotationY);
    Vec3 step = MathUtils::scale(forward, move * speed * deltaTime);

    m_transform.translate(step.x, 0.0f, step.z);

    m_transform.positionX = std::max(-m_worldLimit, std::min(m_worldLimit, m_transform.positionX));
    m_transform.positionZ = std::max(-m_worldLimit, std::min(m_worldLimit, m_transform.positionZ));
}

void Player::drawPart(Renderer& renderer, const float parent[16], const Transform& local, const Vec3& color) const
{
    // Hierarquia de transformacoes: mundo = pai * local
    float localMatrix[16];
    float worldMatrix[16];

    local.getMatrix(localMatrix);
    MathUtils::multiply(parent, localMatrix, worldMatrix);

    renderer.draw(m_cube, worldMatrix, color);
}

void Player::draw(Renderer& renderer) const
{
    float root[16];
    m_transform.getMatrix(root);

    Transform body;
    body.positionY = 0.5f;
    body.scaleX = 0.6f;
    body.scaleY = 1.0f;
    body.scaleZ = 0.4f;

    Transform head;
    head.positionY = 1.25f;
    head.scaleX = 0.45f;
    head.scaleY = 0.45f;
    head.scaleZ = 0.45f;

    Transform nose;
    nose.positionY = 1.25f;
    nose.positionZ = -0.27f;
    nose.scaleX = 0.12f;
    nose.scaleY = 0.12f;
    nose.scaleZ = 0.12f;

    drawPart(renderer, root, body, { 0.95f, 0.45f, 0.35f }); // vestido coral
    drawPart(renderer, root, head, { 0.80f, 0.58f, 0.42f }); // pele
    drawPart(renderer, root, nose, { 0.25f, 0.15f, 0.10f }); // indica a frente
}

Vec3 Player::getPosition() const
{
    return { m_transform.positionX, m_transform.positionY, m_transform.positionZ };
}

float Player::getYaw() const
{
    return m_transform.rotationY;
}

void Player::setGroundHeight(float height)
{
    m_transform.positionY = height;
}
