#ifndef WATER_H
#define WATER_H

// Funcao das ondas da agua. A mesma formula esta no vertex shader
// (assets/shaders/basic.vert): o C++ usa para o barco acompanhar as ondas.
// Se mudar uma, mude a outra.

#include "MathUtils.h"

#include <cmath>

namespace Water {

inline float height(float x, float z, float t)
{
    return 0.060f * std::sin(0.35f * x + 1.1f * t) +
           0.050f * std::sin(0.27f * z - 0.9f * t + 1.3f) +
           0.025f * std::sin(0.90f * (x + z) + 2.0f * t);
}

// Derivadas da altura em x e z (inclinacao da superficie).
inline void slope(float x, float z, float t, float& dx, float& dz)
{
    float shared = 0.025f * 0.90f * std::cos(0.90f * (x + z) + 2.0f * t);
    dx = 0.060f * 0.35f * std::cos(0.35f * x + 1.1f * t) + shared;
    dz = 0.050f * 0.27f * std::cos(0.27f * z - 0.9f * t + 1.3f) + shared;
}

}

#endif
