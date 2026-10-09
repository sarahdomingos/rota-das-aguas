#ifndef LIGHT_H
#define LIGHT_H

// Iluminacao da cena (modelo de Phong): uma luz direcional (o sol) e uma luz
// ambiente "hemisferica" (ceu em cima, chao embaixo), alem da neblina.
// Cada cena define a sua, para ter a atmosfera propria de cada cidade.

#include "MathUtils.h"

struct Light {
    Vec3 sunDirection;   // aponta da superficie para o sol
    Vec3 sunColor;
    Vec3 skyAmbient;
    Vec3 groundAmbient;
    Vec3 fogColor;
    float fogDensity;
};

#endif
