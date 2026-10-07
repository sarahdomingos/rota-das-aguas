#include "MathUtils.h"

#include <cmath>

namespace MathUtils {

float toRadians(float degrees)
{
    return degrees * PI / 180.0f;
}

Vec3 add(const Vec3& a, const Vec3& b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 subtract(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 scale(const Vec3& v, float s)
{
    return { v.x * s, v.y * s, v.z * s };
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 normalize(const Vec3& v)
{
    float length = std::sqrt(dot(v, v));

    if (length < 0.000001f) {
        return { 0.0f, 0.0f, 0.0f };
    }

    return scale(v, 1.0f / length);
}

Vec3 forwardFromYaw(float yawDegrees)
{
    float radians = toRadians(yawDegrees);

    // Mesmo resultado de aplicar a rotacao Y da Transform em (0, 0, -1).
    return { -std::sin(radians), 0.0f, -std::cos(radians) };
}

float angleDifference(float fromDegrees, float toDegrees)
{
    float difference = std::fmod(toDegrees - fromDegrees, 360.0f);

    if (difference > 180.0f) {
        difference -= 360.0f;
    }
    else if (difference < -180.0f) {
        difference += 360.0f;
    }

    return difference;
}

void identity(float matrix[16])
{
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }

    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

void multiply(const float a[16], const float b[16], float result[16])
{
    float temp[16];

    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            temp[column * 4 + row] =
                a[0 * 4 + row] * b[column * 4 + 0] +
                a[1 * 4 + row] * b[column * 4 + 1] +
                a[2 * 4 + row] * b[column * 4 + 2] +
                a[3 * 4 + row] * b[column * 4 + 3];
        }
    }

    for (int i = 0; i < 16; ++i) {
        result[i] = temp[i];
    }
}

// Equivalente ao gluPerspective.
void perspective(float fovYDegrees, float aspect, float zNear, float zFar, float matrix[16])
{
    float f = 1.0f / std::tan(toRadians(fovYDegrees) * 0.5f);

    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }

    matrix[0] = f / aspect;
    matrix[5] = f;
    matrix[10] = (zFar + zNear) / (zNear - zFar);
    matrix[11] = -1.0f;
    matrix[14] = (2.0f * zFar * zNear) / (zNear - zFar);
}

// Equivalente ao gluLookAt.
void lookAt(const Vec3& eye, const Vec3& target, const Vec3& up, float matrix[16])
{
    Vec3 forward = normalize(subtract(target, eye));
    Vec3 right = normalize(cross(forward, up));
    Vec3 cameraUp = cross(right, forward);

    identity(matrix);

    matrix[0] = right.x;
    matrix[4] = right.y;
    matrix[8] = right.z;

    matrix[1] = cameraUp.x;
    matrix[5] = cameraUp.y;
    matrix[9] = cameraUp.z;

    matrix[2] = -forward.x;
    matrix[6] = -forward.y;
    matrix[10] = -forward.z;

    matrix[12] = -dot(right, eye);
    matrix[13] = -dot(cameraUp, eye);
    matrix[14] = dot(forward, eye);
}

}
