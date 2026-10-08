#ifndef CAMERA_H
#define CAMERA_H

// Camera em perspectiva 3D em terceira pessoa que orbita ao redor do alvo (a Lia).
// O mouse controla a orbita (yaw/pitch) e a roda controla a distancia (zoom).

#include "MathUtils.h"

#include <functional>

class Camera {
public:
    Camera();

    void setAspect(float aspect);

    void orbit(float deltaYawDegrees, float deltaPitchDegrees);
    void zoom(float steps);

    // Posiciona imediatamente (sem suavizacao).
    void snapTo(const Vec3& target, float yawDegrees);

    // Segue o alvo suavemente; groundHeight evita que a camera entre no chao.
    void update(const Vec3& target, float deltaTime, const std::function<float(float, float)>& groundHeight);

    float getYaw() const;
    Vec3 getPosition() const;

    // Raio que sai da camera passando pelo ponto da tela (coordenadas -1 a 1).
    void screenRay(float ndcX, float ndcY, Vec3& origin, Vec3& direction) const;

    void getViewMatrix(float matrix[16]) const;
    void getProjectionMatrix(float matrix[16]) const;

private:
    Vec3 m_position;
    Vec3 m_target;
    Vec3 m_smoothTarget;

    float m_yaw;
    float m_pitch;
    float m_distance;
    float m_targetDistance;

    float m_aspect;
    float m_fovY;
    float m_lookHeight;

    void place();
};

#endif
