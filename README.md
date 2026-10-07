# rota-das-aguas
Game 3D utilizando OpenGL 2.0. Contexto ambientado por elementos da cultura alagoana e o ensino da matemática para crianças do ensino fundamental.

# Como executar o projeto
- Clone o repositório localmente;
- Use um **MinGW 32-bit** (ex.: `C:\MinGW\bin`) no início do PATH: a GLFW em `external/lib` é 32-bit e não linka com um g++ 64-bit (o `build.bat` avisa se for o caso);
- Na raiz do projeto, execute o comando `build.bat` (compila, copia as DLLs e abre o jogo).

Alternativa com CMake (mesmo MinGW 32-bit):

```
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=C:/MinGW/bin/g++.exe -DCMAKE_C_COMPILER=C:/MinGW/bin/gcc.exe
cmake --build build
build\RotaDasAguas.exe
```

# Capítulo 1 — Maceió
A cena atual é a orla de Maceió, renderizada com shaders GLSL 1.10 (`assets/shaders`), iluminação Phong, neblina e texturas geradas por código. Tem areia, calçadão, casario colorido, píer de madeira, coqueiros que balançam, pedras, guarda-sóis e o barco Mundaú atracado, acompanhando as ondas. A Lia é um modelo low-poly gerado por código, com braços e pernas animados. Música e efeitos sonoros também são gerados por código (veja `CREDITS.md`).

# Controles
- W A S D / setas → andar (relativo à câmera)
- Shift → correr
- Arrastar com o mouse (botão esquerdo ou direito) → girar a câmera
- Roda do mouse → zoom
- Clique ou E → interagir com o objeto em destaque (barco, placa, coqueiros)
- R / T → girar o barco · Z / X → diminuir / aumentar o barco
- E, Espaço ou clique → fecha a mensagem na tela
- Esc ou P → pausa (na pausa: Q sai do jogo)
- F2 → teste de transição (escurece e volta) · F3 → teste de desafio
- M → liga/desliga a música
- F12 → salva uma captura (`captura.bmp`)

# Organização do código
- `src/core` → `Game` (loop principal), `Window`, `Input`, `AssetPath`
- `src/graphics` → `Shader`, `Renderer` (materiais), `Camera` (orbital), `Mesh` (primitivas), `Model` (.obj), `Texture`, `ProceduralTextures`, `Light`, `Water`, `Transform`, `MathUtils`, `GLFunctions`
- `src/models` → geradores de modelos reutilizáveis: `LiaModel`, `CoastalModels` (coqueiro, pedra, nuvem, guarda-sol), `UrbanModels` (casa, píer, poste, placa)
- `src/scenes` → `Scene` (interface de cada cidade) e `Maceio/MaceioScene`
- `src/entities` → `Player` (Lia), `Boat` (Mundaú)
- `src/ui` → `TextRenderer` (texto na tela com acentos, caixas de diálogo)
- `src/audio` → `AudioEngine` (mixer sobre o waveOut/winmm) e `SoundSynth` (música e efeitos sintetizados)
- `assets/shaders` → `basic.vert`, `basic.frag`
