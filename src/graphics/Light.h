#ifndef LIGHT_H
#define LIGHT_H

// Iluminacao da cena (modelo de Phong): uma luz direcional (o sol, ou a lua a
// noite), uma luz ambiente "hemisferica" (ceu em cima, chao embaixo), a neblina
// e ate MAX_LAMPS luzes pontuais (os postes, que a Lia acende e apaga).
// Cada cena define a sua e pode muda-la a cada quadro (ciclo do dia).

#include "MathUtils.h"

struct Light {
    static const int MAX_LAMPS = 10;

    Vec3 sunDirection;   // aponta da superficie para o sol (ou para a lua)
    Vec3 sunColor;
    Vec3 skyAmbient;
    Vec3 groundAmbient;
    Vec3 fogColor;
    float fogDensity;

    // Luzes pontuais: posicao da lampada e intensidade (0 = apagada, 1 = acesa)
    Vec3 lampColor = { 1.6f, 1.2f, 0.7f };
    Vec3 lampPositions[MAX_LAMPS];
    float lampIntensity[MAX_LAMPS];
    int lampCount = 0;
};

#endif
