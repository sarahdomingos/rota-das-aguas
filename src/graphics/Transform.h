#ifndef TRANSFORM_H
#define TRANSFORM_H

struct Transform {
    float positionX;
    float positionY;
    float positionZ;

    float rotationX;
    float rotationY;
    float rotationZ;

    float scaleX;
    float scaleY;
    float scaleZ;

    Transform();

    // Ao invés de usar as libs padrão de translate, rotate e scale, criamos as nossas próprias funções
    // considerando uma matriz de transformação 4x4
    void translate(float x, float y, float z);
    void rotate(float x, float y, float z);
    void scale(float x, float y, float z);

    void getMatrix(float matrix[16]) const;
};

#endif