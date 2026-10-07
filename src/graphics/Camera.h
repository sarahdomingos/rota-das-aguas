#ifndef CAMERA_H
#define CAMERA_H

// Camera em perspectiva 3D, em terceira pessoa: fica atras e acima do alvo
// (a Lia) e acompanha a posicao e a direcao dele suavemente.

#include "MathUtils.h"

class Camera {
public:
    Camera();

    void setAspect(float aspect);

    // Posiciona a camera imediatamente atras do alvo (sem suavizacao).
    void snapTo(const Vec3& targetPosition, float targetYawDegrees);

    // Aproxima a camera da posicao ideal atras do alvo a cada quadro.
    void follow(const Vec3& targetPosition, float targetYawDegrees, float deltaTime);

    void getViewMatrix(float matrix[16]) const;
    void getProjectionMatrix(float matrix[16]) const;

private:
    Vec3 m_position;
    Vec3 m_target;
    float m_yaw;

    float m_aspect;
    float m_fovY;
    float m_distance;
    float m_height;
    float m_lookHeight;
    float m_followSpeed;

    void place(const Vec3& targetPosition);
};

#endif
