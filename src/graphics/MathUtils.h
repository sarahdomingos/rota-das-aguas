#ifndef MATH_UTILS_H
#define MATH_UTILS_H

// Matematica minima do prototipo: vetores 3D e matrizes 4x4 em ordem
// de coluna (column-major), o mesmo formato do OpenGL e da struct Transform.

struct Vec3 {
    float x;
    float y;
    float z;
};

namespace MathUtils {

const float PI = 3.14159265358979323846f;

float toRadians(float degrees);

Vec3 add(const Vec3& a, const Vec3& b);
Vec3 subtract(const Vec3& a, const Vec3& b);
Vec3 scale(const Vec3& v, float s);
Vec3 cross(const Vec3& a, const Vec3& b);
float dot(const Vec3& a, const Vec3& b);
Vec3 normalize(const Vec3& v);

// Direcao "para frente" (eixo -Z local) depois de girar yawDegrees em torno de Y.
Vec3 forwardFromYaw(float yawDegrees);

// Menor diferenca entre dois angulos, em graus, no intervalo [-180, 180].
float angleDifference(float fromDegrees, float toDegrees);

void identity(float matrix[16]);
void multiply(const float a[16], const float b[16], float result[16]);
void perspective(float fovYDegrees, float aspect, float zNear, float zFar, float matrix[16]);
void lookAt(const Vec3& eye, const Vec3& target, const Vec3& up, float matrix[16]);

}

#endif
