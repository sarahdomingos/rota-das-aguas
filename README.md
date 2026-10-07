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

# Protótipo OpenGL (shaders)
O protótipo renderiza com shaders GLSL 1.10 (`assets/shaders`) e matrizes próprias de modelo, visão e projeção:
- plano de água quadriculado, com uma ilha de areia no centro;
- Lia, representada por cubos (corpo, cabeça e um "nariz" que indica a frente);
- o barco Mundaú (`assets/models/barco_mundau.obj`), balançando na água;
- câmera em perspectiva, em terceira pessoa, seguindo a Lia.

# Controles
- W / ↑ → andar para frente
- S / ↓ → andar para trás
- A / ← e D / → → girar a Lia
- Shift → correr
- Q / E → girar o barco
- Z / X → diminuir / aumentar o barco
- Esc → sair

# Organização do código
- `src/core` → `Game` (loop principal), `Window`, `Input`, `AssetPath`
- `src/graphics` → `Shader`, `Renderer`, `Camera`, `Mesh`, `Model`, `Transform`, `MathUtils`, `GLFunctions`
- `src/entities` → `Entity`, `Player` (Lia), `Boat` (Mundaú)
- `assets/shaders` → `basic.vert`, `basic.frag`
