#ifndef URBAN_MODELS_H
#define URBAN_MODELS_H

// Geradores de construcoes low-poly: casas, pier, poste e placa.
// Reaproveitaveis nas outras cidades do jogo.

#include "graphics/Mesh.h"

namespace UrbanModels {

enum class RoofStyle {
    Gable,      // telhado de duas aguas aparente
    Parapet     // platibanda na fachada (comum no casario historico)
};

struct HouseDescription {
    float width;    // ao longo de X
    float depth;    // ao longo de Z
    float height;   // ate o topo da parede
    int floors;
    Vec3 wallColor;
    Vec3 trimColor;
    Vec3 doorColor;
    Vec3 roofColor;
    RoofStyle roof;
};

// Paredes, portas e janelas (textura de reboco). Fachada virada para -Z,
// base centrada na origem em y = 0.
Mesh houseWalls(const HouseDescription& house);

// Telhado (desenhado com a mesma textura, outra cor de material).
Mesh houseRoof(const HouseDescription& house);

// Pier de madeira ao longo de -Z a partir da origem (topo do deck em deckHeight).
// railGapStart/railGapEnd: trecho (em z local, negativo) sem guarda-corpo do lado +X.
Mesh pier(float width, float length, float deckHeight, float railGapStart, float railGapEnd);

// Poste de iluminacao da orla (base em y = 0).
Mesh streetLamp();

// Placa de madeira com moldura (base em y = 0), virada para +Z.
Mesh woodenSign();

}

#endif
