#ifndef PROCEDURAL_TEXTURES_H
#define PROCEDURAL_TEXTURES_H

// Geradores de texturas por codigo (sem arquivos de imagem).
// Todas sao "repetiveis" (as bordas se encaixam) e em tons claros/neutros:
// a cor final vem da cor do vertice ou do material, que multiplica a textura.
// Podem ser reaproveitadas pelas outras cenas do jogo.

#include <vector>

namespace ProceduralTextures {

const int SIZE = 256;

std::vector<unsigned char> sand();
std::vector<unsigned char> wood();
std::vector<unsigned char> water();
std::vector<unsigned char> plaster();
std::vector<unsigned char> stone();

}

#endif
