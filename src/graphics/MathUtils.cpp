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

// Equivalente ao glOrtho.
void orthographic(float left, float right, float bottom, float top, float zNear, float zFar, float matrix[16])
{
    identity(matrix);
    matrix[0] = 2.0f / (right - left);
    matrix[5] = 2.0f / (top - bottom);
    matrix[10] = -2.0f / (zFar - zNear);
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(zFar + zNear) / (zFar - zNear);
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

float clamp(float value, float minValue, float maxValue)
{
    return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float smoothstep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float length(const Vec3& v)
{
    return std::sqrt(dot(v, v));
}

Vec3 lerp(const Vec3& a, const Vec3& b, float t)
{
    return { lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t) };
}

void translation(float x, float y, float z, float matrix[16])
{
    identity(matrix);
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void rotationX(float degrees, float matrix[16])
{
    identity(matrix);
    float c = std::cos(toRadians(degrees));
    float s = std::sin(toRadians(degrees));
    matrix[5] = c;
    matrix[6] = s;
    matrix[9] = -s;
    matrix[10] = c;
}

void rotationY(float degrees, float matrix[16])
{
    identity(matrix);
    float c = std::cos(toRadians(degrees));
    float s = std::sin(toRadians(degrees));
    matrix[0] = c;
    matrix[2] = -s;
    matrix[8] = s;
    matrix[10] = c;
}

void rotationZ(float degrees, float matrix[16])
{
    identity(matrix);
    float c = std::cos(toRadians(degrees));
    float s = std::sin(toRadians(degrees));
    matrix[0] = c;
    matrix[1] = s;
    matrix[4] = -s;
    matrix[5] = c;
}

void scaling(float x, float y, float z, float matrix[16])
{
    identity(matrix);
    matrix[0] = x;
    matrix[5] = y;
    matrix[10] = z;
}

void apply(float result[16], const float m[16])
{
    multiply(result, m, result);
}

Vec3 transformPoint(const float m[16], const Vec3& p)
{
    return {
        m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12],
        m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13],
        m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]
    };
}

Vec3 transformDirection(const float m[16], const Vec3& d)
{
    return {
        m[0] * d.x + m[4] * d.y + m[8] * d.z,
        m[1] * d.x + m[5] * d.y + m[9] * d.z,
        m[2] * d.x + m[6] * d.y + m[10] * d.z
    };
}

namespace {

// Inversa da parte 3x3 (coluna-major) de m; retorna false se for singular.
bool inverse3x3(const float m[16], float inv[9])
{
    float a = m[0], b = m[4], c = m[8];
    float d = m[1], e = m[5], f = m[9];
    float g = m[2], h = m[6], i = m[10];

    float A = e * i - f * h;
    float B = -(d * i - f * g);
    float C = d * h - e * g;
    float det = a * A + b * B + c * C;

    if (std::fabs(det) < 1e-12f) {
        return false;
    }

    float k = 1.0f / det;

    // inv em ordem linha-major: inv[linha*3 + coluna]
    inv[0] = A * k;
    inv[1] = -(b * i - c * h) * k;
    inv[2] = (b * f - c * e) * k;
    inv[3] = B * k;
    inv[4] = (a * i - c * g) * k;
    inv[5] = -(a * f - c * d) * k;
    inv[6] = C * k;
    inv[7] = -(a * h - b * g) * k;
    inv[8] = (a * e - b * d) * k;

    return true;
}

}

void normalMatrix(const float model[16], float result[16])
{
    float inv[9];
    identity(result);

    if (!inverse3x3(model, inv)) {
        return;
    }

    // Transposta da inversa: elemento (linha r, coluna c) = inv(c, r)
    for (int column = 0; column < 3; ++column) {
        for (int row = 0; row < 3; ++row) {
            result[column * 4 + row] = inv[column * 3 + row];
        }
    }
}

void inverseAffine(const float m[16], float result[16])
{
    float inv[9];
    identity(result);

    if (!inverse3x3(m, inv)) {
        return;
    }

    for (int column = 0; column < 3; ++column) {
        for (int row = 0; row < 3; ++row) {
            result[column * 4 + row] = inv[row * 3 + column];
        }
    }

    Vec3 t = { m[12], m[13], m[14] };
    result[12] = -(result[0] * t.x + result[4] * t.y + result[8] * t.z);
    result[13] = -(result[1] * t.x + result[5] * t.y + result[9] * t.z);
    result[14] = -(result[2] * t.x + result[6] * t.y + result[10] * t.z);
}

}
