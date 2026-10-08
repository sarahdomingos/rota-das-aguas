#include "Camera.h"

#include <cmath>

using namespace MathUtils;

namespace {

const float MIN_PITCH = 5.0f;
const float MAX_PITCH = 70.0f;
const float MIN_DISTANCE = 2.5f;
const float MAX_DISTANCE = 16.0f;

}

Camera::Camera()
    : m_position{ 0.0f, 3.0f, 6.0f },
      m_target{ 0.0f, 1.0f, 0.0f },
      m_smoothTarget{ 0.0f, 1.0f, 0.0f },
      m_yaw(0.0f),
      m_pitch(20.0f),
      m_distance(6.5f),
      m_targetDistance(6.5f),
      m_aspect(16.0f / 9.0f),
      m_fovY(55.0f),
      m_lookHeight(1.0f)
{
}

void Camera::setAspect(float aspect)
{
    if (aspect > 0.0f) {
        m_aspect = aspect;
    }
}

void Camera::orbit(float deltaYawDegrees, float deltaPitchDegrees)
{
    m_yaw += deltaYawDegrees;
    m_pitch = clamp(m_pitch + deltaPitchDegrees, MIN_PITCH, MAX_PITCH);
}

void Camera::zoom(float steps)
{
    m_targetDistance = clamp(m_targetDistance * std::pow(0.88f, steps), MIN_DISTANCE, MAX_DISTANCE);
}

void Camera::place()
{
    Vec3 forward = forwardFromYaw(m_yaw);
    float pitch = toRadians(m_pitch);

    m_target = add(m_smoothTarget, { 0.0f, m_lookHeight, 0.0f });

    m_position = subtract(m_target, scale(forward, m_distance * std::cos(pitch)));
    m_position.y += m_distance * std::sin(pitch);
}

void Camera::snapTo(const Vec3& target, float yawDegrees)
{
    m_yaw = yawDegrees;
    m_smoothTarget = target;
    m_distance = m_targetDistance;
    place();
}

void Camera::update(const Vec3& target, float deltaTime, const std::function<float(float, float)>& groundHeight)
{
    // Suavizacao exponencial: independe da taxa de quadros.
    float follow = 1.0f - std::exp(-10.0f * deltaTime);
    float zooming = 1.0f - std::exp(-8.0f * deltaTime);

    m_smoothTarget = lerp(m_smoothTarget, target, follow);
    m_distance = lerp(m_distance, m_targetDistance, zooming);

    place();

    // Nao deixa a camera atravessar o chao nem a agua
    float minimumHeight = std::fmax(groundHeight(m_position.x, m_position.z), 0.0f) + 0.4f;
    if (m_position.y < minimumHeight) {
        m_position.y = minimumHeight;
    }
}

float Camera::getYaw() const
{
    return m_yaw;
}

Vec3 Camera::getPosition() const
{
    return m_position;
}

void Camera::screenRay(float ndcX, float ndcY, Vec3& origin, Vec3& direction) const
{
    Vec3 forward = normalize(subtract(m_target, m_position));
    Vec3 right = normalize(cross(forward, { 0.0f, 1.0f, 0.0f }));
    Vec3 up = cross(right, forward);
    float tanHalf = std::tan(toRadians(m_fovY) * 0.5f);

    origin = m_position;
    direction = normalize(add(forward, add(scale(right, ndcX * tanHalf * m_aspect), scale(up, ndcY * tanHalf))));
}

void Camera::getViewMatrix(float matrix[16]) const
{
    lookAt(m_position, m_target, { 0.0f, 1.0f, 0.0f }, matrix);
}

void Camera::getProjectionMatrix(float matrix[16]) const
{
    perspective(m_fovY, m_aspect, 0.1f, 400.0f, matrix);
}
