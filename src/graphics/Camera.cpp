#include "Camera.h"

#include <cmath>

Camera::Camera()
    : m_position{ 0.0f, 3.0f, 6.0f },
      m_target{ 0.0f, 1.0f, 0.0f },
      m_yaw(0.0f),
      m_aspect(16.0f / 9.0f),
      m_fovY(60.0f),
      m_distance(6.0f),
      m_height(3.0f),
      m_lookHeight(1.2f),
      m_followSpeed(4.0f)
{
}

void Camera::setAspect(float aspect)
{
    if (aspect > 0.0f) {
        m_aspect = aspect;
    }
}

void Camera::place(const Vec3& targetPosition)
{
    Vec3 forward = MathUtils::forwardFromYaw(m_yaw);

    m_target = MathUtils::add(targetPosition, { 0.0f, m_lookHeight, 0.0f });

    m_position = MathUtils::subtract(targetPosition, MathUtils::scale(forward, m_distance));
    m_position.y += m_height;
}

void Camera::snapTo(const Vec3& targetPosition, float targetYawDegrees)
{
    m_yaw = targetYawDegrees;
    place(targetPosition);
}

void Camera::follow(const Vec3& targetPosition, float targetYawDegrees, float deltaTime)
{
    // Suavizacao exponencial: independe da taxa de quadros.
    float blend = 1.0f - std::exp(-m_followSpeed * deltaTime);

    m_yaw += MathUtils::angleDifference(m_yaw, targetYawDegrees) * blend;

    place(targetPosition);
}

void Camera::getViewMatrix(float matrix[16]) const
{
    MathUtils::lookAt(m_position, m_target, { 0.0f, 1.0f, 0.0f }, matrix);
}

void Camera::getProjectionMatrix(float matrix[16]) const
{
    MathUtils::perspective(m_fovY, m_aspect, 0.1f, 200.0f, matrix);
}
