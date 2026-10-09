#include "Transform.h"

#include <cmath>

namespace {

const float PI = 3.14159265358979323846f;

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

void multiply(
    const float a[16],
    const float b[16],
    float result[16])
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

// Transformação de matrizes manuais para os objetos de cena
void createTranslation(
    float x,
    float y,
    float z,
    float matrix[16])
{
    identity(matrix);

    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void createScale(
    float x,
    float y,
    float z,
    float matrix[16])
{
    identity(matrix);

    matrix[0] = x;
    matrix[5] = y;
    matrix[10] = z;
}

void createRotationX(
    float degrees,
    float matrix[16])
{
    identity(matrix);

    float radians =
        degrees * PI / 180.0f;

    float c = std::cos(radians);
    float s = std::sin(radians);

    matrix[5] = c;
    matrix[6] = s;
    matrix[9] = -s;
    matrix[10] = c;
}

void createRotationY(
    float degrees,
    float matrix[16])
{
    identity(matrix);

    float radians =
        degrees * PI / 180.0f;

    float c = std::cos(radians);
    float s = std::sin(radians);

    matrix[0] = c;
    matrix[2] = -s;
    matrix[8] = s;
    matrix[10] = c;
}

void createRotationZ(
    float degrees,
    float matrix[16])
{
    identity(matrix);

    float radians =
        degrees * PI / 180.0f;

    float c = std::cos(radians);
    float s = std::sin(radians);

    matrix[0] = c;
    matrix[1] = s;
    matrix[4] = -s;
    matrix[5] = c;
}

}

Transform::Transform()
    : positionX(0.0f),
      positionY(0.0f),
      positionZ(0.0f),
      rotationX(0.0f),
      rotationY(0.0f),
      rotationZ(0.0f),
      scaleX(1.0f),
      scaleY(1.0f),
      scaleZ(1.0f)
{
}

void Transform::translate(
    float x,
    float y,
    float z)
{
    positionX += x;
    positionY += y;
    positionZ += z;
}

void Transform::rotate(
    float x,
    float y,
    float z)
{
    rotationX += x;
    rotationY += y;
    rotationZ += z;
}

void Transform::scale(
    float x,
    float y,
    float z)
{
    scaleX *= x;
    scaleY *= y;
    scaleZ *= z;
}

void Transform::getMatrix(float matrix[16]) const
{
    float translation[16];
    float rotationXMatrix[16];
    float rotationYMatrix[16];
    float rotationZMatrix[16];
    float scaleMatrix[16];

    float rotationXY[16];
    float rotation[16];
    float rotationScale[16];

    createTranslation(
        positionX,
        positionY,
        positionZ,
        translation
    );

    createRotationX(
        rotationX,
        rotationXMatrix
    );

    createRotationY(
        rotationY,
        rotationYMatrix
    );

    createRotationZ(
        rotationZ,
        rotationZMatrix
    );

    createScale(
        scaleX,
        scaleY,
        scaleZ,
        scaleMatrix
    );

    // R = Rz * Ry * Rx
    multiply(
        rotationYMatrix,
        rotationXMatrix,
        rotationXY
    );

    multiply(
        rotationZMatrix,
        rotationXY,
        rotation
    );

    // R * S
    multiply(
        rotation,
        scaleMatrix,
        rotationScale
    );

    // T * R * S
    multiply(
        translation,
        rotationScale,
        matrix
    );
}