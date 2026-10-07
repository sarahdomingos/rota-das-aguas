# Log de mudanças

Uma entrada por pedido feito no projeto, da mais recente para a mais antiga.

## 2026-10-07 — Cena de Maceió: modelagem, luz, texturas, água, mouse, colisão e som

**Pedido:** evoluir o protótipo para o Capítulo 1 (Maceió). Os itens aprovados foram: modelo low-poly da Lia, cena da orla de Maceió, hierarquia de transformações, colisão, câmera com mouse e interação, som e música, iluminação Phong, texturas e água animada. A água não pode entrar no barco quando ele oscila. A Lia foi simplificada a pedido da usuária (cabelo liso em bloco, sandália simples).

### Adicionado
- **Lia em low-poly**, gerada por código (`src/models/LiaModel`):
  - tem tronco, cabeça com olhos, sobrancelhas, nariz, boca e bochechas, cabelo liso, camiseta amarela, short, mochila com bolso e alças, e sandálias;
  - braços e pernas são peças separadas presas em ombros e quadril (hierarquia de transformações);
  - ela anda, corre, inclina o tronco e "respira" quando está parada.
- **Cena de Maceió** (`src/scenes/Maceio/MaceioScene`), com:
  - terreno facetado: o mar vai ficando fundo, a areia molhada fica junto da água e há dunas na areia seca;
  - calçadão com faixa ondulada e meio-fio;
  - rua de paralelepípedos e calçada;
  - casario colorido, com casas de telhado de duas águas ou platibanda, janelas com venezianas e sobrados com sacadas;
  - píer de madeira com guarda-corpo e uma abertura onde o barco atraca;
  - 15 coqueiros, com as folhas balançando ao vento;
  - pedras com limo na beira d'água;
  - guarda-sóis listrados e postes;
  - uma placa de boas-vindas;
  - céu em degradê, sol e nuvens.
- **Geradores reutilizáveis** para as outras cidades:
  - `src/models/CoastalModels` (coqueiro, pedra, nuvem, guarda-sol);
  - `src/models/UrbanModels` (casa, píer, poste, placa);
  - primitivas novas na `Mesh` (caixa, cilindro ou cone, esfera, grade de terreno, disco, cúpula do céu e junção de malhas com matriz).
- **Interface de cena** `src/scenes/Scene.h`, para que cada cidade tenha seu próprio terreno, colisões, luz e interações.
- **Colisão:**
  - a Lia não atravessa casas, coqueiros, pedras, postes, guarda-sóis, a placa, o guarda-corpo do píer nem o barco;
  - também não entra no mar fundo e desliza ao longo dos obstáculos;
  - ela sobe no píer e no calçadão acompanhando a altura do chão.
- **Mouse e teclado:**
  - arrastar o mouse gira a câmera em volta da Lia e a roda dá zoom;
  - a câmera não atravessa o chão;
  - W/A/S/D andam na direção para onde a câmera olha;
  - clique ou E interage com o objeto mais próximo ou com o objeto clicado (barco, placa, coqueiros). O objeto em foco brilha, e a interação toca um som e mostra uma mensagem no título da janela;
  - o coqueiro balança quando a Lia interage com ele.
- **Iluminação Phong** (Blinn-Phong por pixel): sol direcional, luz ambiente de céu e chão, brilho especular por material e neblina de distância (`src/graphics/Light.h`, `assets/shaders/basic.frag`).
- **Texturas geradas por código** (`src/graphics/ProceduralTextures`, `src/graphics/Texture`): areia, madeira, água, reboco e paralelepípedos, com mipmaps.
- **Barco com textura de madeira:** `assets/models/textura_madeira.bmp`, convertida uma vez do `.jpg` da equipe (512×512), lida por um leitor de BMP próprio.
- **Água animada:**
  - ondas no vertex shader;
  - reflexo do sol;
  - transparência com efeito Fresnel.
- **O barco acompanha as ondas:** sobe, desce e inclina com a mesma fórmula do shader (`src/graphics/Water.h`).
- **Água fora do casco:** o shader descarta a água dentro do contorno do casco, medido no modelo, e a borda do barco fica sempre acima das ondas.
- **Sombras simples** (discos projetados na direção oposta ao sol) embaixo da Lia, dos coqueiros e dos guarda-sóis.
- **Som e música gerados por código**, sem arquivos de terceiros (`src/audio`):
  - mixer próprio sobre o waveOut do Windows (winmm), com thread do Win32;
  - música em loop no clima de baião (sanfona, baixo, zabumba e triângulo);
  - ondas do mar, com volume maior perto da água;
  - passos diferentes na areia, na madeira e na pedra;
  - sino de interação e folhas balançando;
  - M liga e desliga a música.
- **Captura de tela:** F12 salva `captura.bmp`. Para testes, `ROTA_AUTOSHOT=segundos` salva `autoshot.bmp` e fecha o jogo, sem tomar o foco do teclado, e `ROTA_START="x z giroLia giroCamera"` define a posição inicial.
- `CREDITS.md` com a origem de cada recurso.

### Alterado
- `Camera`: passou a orbitar a Lia (yaw, pitch e zoom) em vez de segui-la por trás. Também calcula o raio do mouse, usado no clique.
- `Input`: guarda o estado do quadro anterior (tecla ou botão "acabou de ser apertado"), o movimento do mouse e a roda.
- `Renderer`: ganhou materiais (cor, textura, brilho, destaque, transparência, água, dupla face), céu, passe de transparência, matriz de normais e a máscara do barco na água.
- `Shader`: guarda a posição dos uniforms em cache e ganhou `setFloat` e `setInt`. Novas funções do OpenGL 2.0 em `GLFunctions`: `glUniform1f` e `glUniform1i`.
- `Mesh`: agora tem coordenadas de textura.
- `MathUtils`: ganhou matrizes de rotação, escala e translação, inversa, matriz de normais e interpolação.
- `Player` e `Boat` foram reescritos para a cena nova. Os controles do barco mudaram: **R/T giram** (E agora é interagir) e Z/X continuam mudando a escala.
- `Model` (carregador .obj): a struct `Material` virou `ModelMaterial`, para não conflitar com o material do `Renderer`.
- `build.bat` e `CMakeLists.txt`: linkam a `winmm` (áudio).
- `README.md`: controles e organização atualizados.

### Removido
- `src/graphics/Light.cpp` (vazio): a luz agora é a struct `Light` em `Light.h`.
- `src/scenes/MaceioScene.h/.cpp` (vazios): a cena foi para `src/scenes/Maceio/`.
- O cubo e o plano quadriculado do protótipo anterior.

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
