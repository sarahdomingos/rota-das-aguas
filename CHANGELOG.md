# Log de mudanças

Uma entrada por pedido feito no projeto, da mais recente para a mais antiga.

## 2026-10-07 — Protótipo OpenGL (Fase 2 / Etapa 1 da evolução gráfica)

**Pedido:** implementar o protótipo OpenGL do jogo: janela com loop principal e teclado, shaders básicos, primitivas (água, Lia, barco Mundaú), transformações com matrizes, movimentação da Lia por teclado e câmera em perspectiva em terceira pessoa. Feito na branch `prototipo-opengl`, criada a partir de `development`.

### Adicionado
- **Shaders** `assets/shaders/basic.vert` e `assets/shaders/basic.frag` (GLSL 1.10, compatível com OpenGL 2.0), com matrizes de modelo, visão e projeção e uma cor por objeto.
- **Água:** plano quadriculado em dois tons de azul (90 × 90 unidades), para dar noção de movimento.
- **Ilha de areia** no centro do mapa (cubo achatado), onde a Lia começa.
- **Lia:** personagem feita de cubos (corpo coral, cabeça e um "nariz" que mostra a frente), desenhados com hierarquia de transformações.
- **Barco Mundaú** na água, usando o modelo `barco_mundau.obj` existente, com balanço automático (translação vertical e rotação leve). Se o `.obj` não carregar, um casco simples gerado por código é desenhado no lugar.
- **Câmera em terceira pessoa:** fica atrás e acima da Lia e acompanha a posição e a direção dela de forma suave.
- **Controles:** W/S ou ↑/↓ andam, A/D ou ←/→ giram a Lia, Shift corre, Q/E giram e Z/X mudam a escala do barco, Esc sai.
- `src/core/Game.h/.cpp`: loop principal (delta time, entrada, atualização e desenho).
- `src/core/AssetPath.h/.cpp`: encontra os arquivos de `assets/` rodando tanto da raiz quanto de `build/`.
- `src/graphics/Shader.h/.cpp`: carrega, compila e linka os shaders, com mensagens de erro.
- `src/graphics/GLFunctions.h/.cpp`: carrega as funções de shader do OpenGL 2.0 via GLFW, sem precisar de GLAD/GLEW.
- `src/graphics/MathUtils.h/.cpp`: vetores 3D e matrizes 4x4 (multiplicação, perspectiva e lookAt).
- `src/graphics/Mesh.h/.cpp`: primitivas (cubo, plano de água e casco de barco) desenhadas com vertex arrays.
- `src/graphics/Camera.h/.cpp`: câmera em perspectiva em terceira pessoa.
- `src/entities/Player.h/.cpp` (Lia) e `src/entities/Boat.h/.cpp` (Mundaú); `src/entities/Entity.h` virou a classe base com `Transform`, `update` e `draw`.
- `CHANGELOG.md` (este arquivo).

### Alterado
- `src/main.cpp`: agora só cria e executa o `Game` (o loop saiu daqui).
- `src/graphics/Renderer.h/.cpp`: passou a usar o shader e a câmera. Desenha qualquer `Mesh` ou `Model` com uma matriz de modelo e uma cor, no lugar de cuidar só do barco.
- `src/core/Window.h/.cpp`: acompanha o redimensionamento da janela (viewport e proporção da câmera), ganhou `close()` e antisserrilhamento.
- Os controles do barco (Q/E e Z/X) foram mantidos. As setas, que antes moviam o barco, agora movem a Lia.
- Pasta `src/entinties` renomeada para `src/entities`, como na estrutura da spec.
- `build.bat`: adiciona `-Isrc`, linka o runtime do C++ estaticamente, copia a `libgcc_s_dw2-1.dll` (exigida pela `glfw3.dll`), avisa quando o g++ do PATH é 64-bit e executa `.\jogo.exe`.
- `CMakeLists.txt`: no MinGW, linka o runtime do C++ estaticamente e copia a `libgcc_s_dw2-1.dll` para a pasta do executável.
- `README.md`: como compilar (MinGW 32-bit), controles e organização do código.

### Removido
- Iluminação de pipeline fixo (`glLight*`) da `Window`: ela não tem efeito quando se usam shaders. A iluminação volta numa etapa futura, nos shaders. Por enquanto o fragment shader só escurece as faces de acordo com a normal, para os volumes ficarem legíveis.
- Câmera fixa com `gluLookAt` e projeção com `glFrustum` do `main.cpp`, substituídas pela `Camera` com matrizes próprias.
