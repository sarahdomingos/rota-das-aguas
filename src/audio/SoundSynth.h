#ifndef SOUND_SYNTH_H
#define SOUND_SYNTH_H

// Sons e musica gerados por codigo (sintese), sem arquivos de audio externos.
// Assim nao ha questao de licenca: tudo e original do projeto.

#include "AudioEngine.h"

namespace SoundSynth {

// Musica ambiente em loop (~20 s), no clima de um baiao: sanfona, baixo,
// zabumba e triangulo, em do maior.
Sound musicMaceio();

// Ondas do mar em loop (~16 s), com "respiros" de arrebentacao.
Sound waves();

Sound footstepSand(int variation);
Sound footstepWood(int variation);
Sound footstepStone(int variation);

// Sino curto de interacao.
Sound chime();

// Folhas do coqueiro balancando.
Sound leavesRustle();

// "Clique" suave ao avancar uma fala do dialogo.
Sound dialogueBlip();

// Notinha subindo ao pegar um item.
Sound pickup();

// Arpejo alegre de acerto (desafio resolvido).
Sound success();

}

#endif
