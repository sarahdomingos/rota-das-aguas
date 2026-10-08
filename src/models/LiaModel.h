#ifndef LIA_MODEL_H
#define LIA_MODEL_H

// Modelo low-poly da Lia gerado por codigo (spec, sec. 6), na versao simples:
// cabelo liso em bloco, camiseta, short, mochila e sandalias.
//
// O modelo e dividido em partes para a hierarquia de transformacoes:
// corpo (tronco + cabeca + cabelo + mochila), dois bracos e duas pernas,
// cada membro com o pivo no ombro/quadril. Ela olha para -Z.

#include "graphics/Mesh.h"

class Renderer;

struct CharacterModel {
    Mesh body;
    Mesh leftArm;
    Mesh rightArm;
    Mesh leftLeg;
    Mesh rightLeg;

    Vec3 leftShoulder;
    Vec3 rightShoulder;
    Vec3 leftHip;
    Vec3 rightHip;
};

namespace LiaModel {

CharacterModel build();

// Desenha a personagem animada.
// walkPhase: fase do ciclo de passos (radianos); walkAmount: 0 parada, 1 andando, >1 correndo.
void draw(const CharacterModel& model, Renderer& renderer, const float root[16],
          float walkPhase, float walkAmount, float time);

}

#endif
