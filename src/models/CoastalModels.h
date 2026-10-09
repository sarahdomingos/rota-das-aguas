#ifndef COASTAL_MODELS_H
#define COASTAL_MODELS_H

// Geradores de modelos low-poly do litoral: coqueiro, pedra, nuvem e guarda-sol.
// Reaproveitaveis em outras cenas (Coqueiro Seco, Marechal Deodoro...).

#include "graphics/Mesh.h"

namespace CoastalModels {

// Tronco curvo do coqueiro (inclinado para +X) com os cocos.
// "top" recebe o ponto onde as folhas devem ser presas.
Mesh palmTrunk(float height, float lean, Vec3& top);

// Uma folha (palha) do coqueiro, saindo da origem ao longo de +X.
// E desenhada varias vezes, girada em volta do topo do tronco.
Mesh palmFrond(float length);

// Pedra irregular apoiada em y = 0.
Mesh rock(int seed, float size);

// Nuvem fofa feita de esferas achatadas.
Mesh cloud(int seed);

// Guarda-sol de praia listrado com mastro (base em y = 0).
Mesh beachUmbrella(const Vec3& colorA, const Vec3& colorB);

}

#endif
