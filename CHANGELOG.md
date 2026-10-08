# Log de mudanças

Uma entrada por pedido feito no projeto, da mais recente para a mais antiga.

## 2026-10-07 — Etapa A7: missões (tarefas)

**Pedido:** etapa A7 do roteiro: tarefas definidas como dados, com objetivo, condição de conclusão (ter N itens ou entregar a um NPC), diálogo de conclusão e próxima tarefa, ligadas a NPCs e diálogos pelo id. O objetivo atual aparece no HUD. Feito na branch `missoes`, criada a partir da `inventario`.

### Adicionado
- `assets/missoes/maceio.txt`: as missões do capítulo, com o formato explicado no topo.
  - Campos: `titulo`, `objetivo`, `quem`, `condicao` (`entregar coco 3` ou `ter coco 3`), `inicial`, `dialogo_inicio`, `dialogo_andamento`, `dialogo_conclusao`, `desafio` e `proxima`.
- `src/gameplay/Quest` (`QuestLog`): carrega as missões e controla o estado de cada uma (bloqueada, disponível, em andamento ou concluída).
  - Ao falar com quem pede, escolhe o diálogo certo: pedido, andamento ou conclusão.
  - Ao fechar o diálogo, inicia ou conclui a missão. Na entrega, retira os itens da mochila e libera a próxima missão.
- **Missão de demonstração "Cocos para a tapioca":** a Dona Graça tem 2 cocos e precisa de 5. A Lia acha 3 cocos pela orla e entrega a ela. Diálogos novos: `graca_pedido`, `graca_andamento` e `graca_conclusao`.
- **HUD da missão** no canto superior esquerdo: título, objetivo e progresso (ex.: "Coco: 1/3").
- **Avisos** "Nova missão" e "Missão concluída", com som.
- Teste: `ROTA_TEST_STATE=missao` abre o jogo com a missão em andamento e 1 coco na mochila.

### Alterado
- A interação com NPCs e objetos passa pelas missões antes de abrir o diálogo. Sem missão, abre o diálogo normal do NPC ou objeto.
- O fim de um diálogo avisa as missões (`Game::onDialogueFinished`).

### Removido
- Nada.

## 2026-10-07 — Etapa A6: inventário e HUD

**Pedido:** etapa A6 do roteiro: inventário que soma e retira quantidades, HUD com os itens que a Lia carrega e uma tela de inventário na tecla I. Feito na branch `inventario`, criada a partir da `itens-coletaveis`.

### Adicionado
- `src/gameplay/Inventory`: a mochila da Lia.
  - `add` e `remove` (retirar só funciona se houver quantidade suficiente, o que já atende as entregas e a subtração do capítulo 2), `count` e `entries`.
  - Os itens ficam na ordem em que foram pegos.
- **HUD** no canto superior direito com os itens carregados (quadradinho na cor do item, nome e quantidade). Só aparece quando há algum item.
- **Tela "Mochila da Lia"** na tecla I: um estado próprio (`GameState::Inventory`) que para o mundo como a pausa e lista nome, categoria e quantidade. Fecha com I ou Esc.
- Teste: `ROTA_TEST_STATE=mochila` abre o jogo com a mochila aberta e alguns itens.

### Alterado
- Pegar um item agora o guarda na mochila (o som e o aviso da A5 continuam).
- As dicas de controle (canto e pausa) incluem "I: mochila".

### Removido
- Nada.

## 2026-10-07 — Etapa A5: itens coletáveis

**Pedido:** etapa A5 do roteiro: itens coletáveis genéricos, definidos como dados. No mundo, o item gira e flutua, brilha em foco e some ao ser pego com clique ou E, com som. Feito na branch `itens-coletaveis`, criada a partir da `modelo-npc`.

### Adicionado
- `assets/itens/maceio.txt`: tipos de item (id, nome, categoria, modelo e cor) e posições no mundo, com o formato explicado no topo. Há 3 cocos, 2 conchas e 1 fita de festa pela orla.
- `src/gameplay/Items`: catálogo de itens (`ItemDefinition`, `ItemCatalog`) e leitor do arquivo (`loadItemsFile`), reaproveitável por qualquer cena.
- `src/entities/Item`: item no mundo.
  - Flutua e gira devagar e tem um brilho leve.
  - Brilha mais quando está em foco e some ao ser pego.
- `src/models/ItemModels`: modelos simples gerados por código (coco, concha e fita de festa).
- **Ao pegar um item:** toca um som de pegar (`SoundSynth::pickup`, três notas subindo) e aparece o aviso "Você pegou: Coco (+1)" no alto da tela por alguns segundos.

### Alterado
- `Scene`: `Interaction` informa o item pego (`itemId`, `itemQuantity`) e a cena expõe o catálogo de itens (`getItemCatalog`).
- `MaceioScene`: carrega os itens do arquivo, desenha os itens e não deixa focar os que já foram pegos.

### Removido
- Nada.

## 2026-10-07 — Etapa A4: modelo de NPC reutilizável

**Pedido:** executar a etapa A4 do roteiro: um gerador de NPC que reaproveita o corpo e a hierarquia da Lia, com aparência definida por parâmetros. Os NPCs respiram parados, viram para a Lia quando ela chega perto, têm colisão e abrem um diálogo da A3. Para demonstrar, dois NPCs em Maceió. Feito na branch `modelo-npc`, criada a partir da `sistema-de-dialogo`.

### Adicionado
- **`CharacterStyle`** (`src/models/LiaModel.h`): a aparência de um personagem.
  - Cores de pele, cabelo, camisa, short e calçado.
  - Estilo de cabelo: comprido, curto ou careca.
  - Acessório: mochila, chapéu de palha, avental ou nenhum, com a cor do acessório.
  - Altura (escala).
- `LiaModel::build(style)` monta qualquer personagem com essa aparência; `LiaModel::liaStyle()` é a aparência da Lia.
- **`src/entities/NPC`**: entidade com nome, aparência, posição, rotação e id de diálogo.
  - Fica parado respirando, com uma fase própria para os NPCs não se mexerem todos juntos.
  - Vira suavemente para a Lia quando ela está a menos de 4,5 unidades e depois volta à posição original.
  - Brilha quando está em foco.
- **Dois NPCs em Maceió**, com colisão, sombra e interação (clique ou E):
  - **Seu Bené**, pescador no píer, de chapéu de palha;
  - **Dona Graça**, vendedora de tapioca no calçadão, de avental.
- **Diálogos novos** `[pescador]` e `[vendedora]` em `assets/dialogos/maceio.txt`, com pequenas contas de divisão e soma.

### Alterado
- `LiaModel`: as cores fixas viraram parâmetros, e `draw` ganhou o destaque (objeto em foco). A Lia continua com a mesma aparência.
- `MaceioScene`: os NPCs entram como objetos interativos (`Kind::Npc`), com círculo de colisão e sombra.
- `README.md`: como criar um NPC.

### Removido
- Nada.

## 2026-10-07 — Etapa A3: sistema de diálogo

**Pedido:** executar a etapa A3 do roteiro: um sistema de diálogo genérico e orientado a dados, em que cada capítulo só precisa de um arquivo novo de falas. As interações do barco, da placa e do coqueiro passam a abrir diálogos desse arquivo. Feito na branch `sistema-de-dialogo`, criada a partir da `estados-do-jogo`.

### Adicionado
- `assets/dialogos/maceio.txt`: as falas do Capítulo 1, em texto simples (UTF-8) e com o formato explicado no topo do arquivo.
  - Cada diálogo é um bloco `[id]` seguido de linhas `Nome: fala`; `Narrador:` é a narração.
  - Diálogos: `inicio` (chegada a Maceió), `placa` (apresenta Maceió e a Festa das Águas), `barco` (o Mundaú) e `coqueiro`.
- `src/gameplay/Dialogue.h/.cpp`: carrega os arquivos de diálogo e toca um diálogo pelo id.
  - As letras aparecem uma a uma (45 por segundo).
  - Apertar enquanto a fala aparece mostra a fala inteira; apertar de novo passa para a próxima, e na última fecha o diálogo.
- **Caixa de diálogo:**
  - o nome de quem fala aparece numa etiqueta amarela em destaque;
  - a narração fica em azul-claro e sem etiqueta;
  - a caixa tem altura fixa, para não "pular" enquanto as letras aparecem;
  - o aviso diz "continuar" ou "fechar" (na última fala).
- **Som curto** ao avançar as falas (`SoundSynth::dialogueBlip`, gerado por código).

### Alterado
- **A interação abre um diálogo:** a interação com barco, placa e coqueiro agora devolve o id do diálogo (`Interaction::dialogueId`), e o `Game` abre esse diálogo no estado DIÁLOGO (A2). Fechar a última fala volta para JOGANDO.
- **Abertura do jogo:** a mensagem de boas-vindas virou o diálogo `inicio`.
- O teste de desafio (F3) usa a mesma caixa, com borda azul e a etiqueta "Desafio".
- `README.md`: como editar os diálogos.

### Removido
- As mensagens de interação escritas direto no código da cena (agora ficam no arquivo de diálogos).

## 2026-10-07 — Etapa A2: estados do jogo

**Pedido:** executar a etapa A2 do roteiro: uma máquina de estados simples no jogo (jogando, diálogo, desafio, pausa e transição), com tela de pausa e transição com escurecimento. Feito na branch `estados-do-jogo`, criada a partir da `texto-na-tela`.

### Adicionado
- **Estados do jogo** (`enum class GameState` em `src/core/Game.h`): JOGANDO, DIÁLOGO, DESAFIO, PAUSA e TRANSIÇÃO. A troca de estado passa por `changeState`, que chama `exitState` do estado antigo e `enterState` do novo.
  - **JOGANDO:** único estado em que a Lia anda, interage e controla o barco.
  - **DIÁLOGO:** a mensagem de interação (e a de boas-vindas) fica na caixa até ser fechada com E, Espaço ou clique. A Lia fica parada, mas a água, os coqueiros e o barco continuam animando, e a câmera ainda pode ser girada.
  - **DESAFIO:** por enquanto é só um teste (F3), com uma caixa de borda azul. Congela a Lia como o diálogo e será usado pelos desafios de matemática.
  - **PAUSA:** o mundo inteiro para. Mostra a tela "Pausa" com os controles; Esc ou P continua e Q sai do jogo. A música fica mais baixa durante a pausa.
  - **TRANSIÇÃO:** escurece até o preto e clareia de volta em 1,2 segundo. Por enquanto é só um teste (F2) e será usada na troca de cena.
- Aviso "E, Espaço ou clique: continuar" na caixa de mensagem.
- Teste: `ROTA_TEST_STATE=pausa` abre o jogo já pausado. A captura automática (`ROTA_AUTOSHOT`) passou a contar o tempo real, para funcionar também na pausa.

### Alterado
- **Esc não fecha mais o jogo direto:** abre e fecha a pausa (P também). Para sair, use Q na pausa.
- A mensagem não some mais sozinha depois de 6 segundos: ela abre o estado de diálogo e espera ser fechada.
- A dica de controles do canto inclui "Esc/P: pausa".
- `TextRenderer`: há um espaço entre os caracteres no atlas da fonte, para não aparecerem pontinhos do caractere vizinho.
- `README.md`: controles novos.

### Removido
- O fechamento do jogo pelo Esc e o temporizador da caixa de mensagem.

## 2026-10-07 — Etapa A1: texto na tela

**Pedido:** executar a próxima etapa do roteiro (A1, texto na tela): fonte com acentos do português desenhada em 2D por cima da cena, caixa de diálogo semitransparente, mensagens de interação fora do título da janela e uma dica discreta de controles. A API precisa ser reutilizável por diálogo, HUD e inventário (etapas A3 e A6). Feito na branch `texto-na-tela`, criada a partir da `prototipo-opengl`.

### Adicionado
- `src/ui/TextRenderer.h/.cpp`: texto na tela reutilizável.
  - A fonte (Segoe UI) é desenhada uma vez pelo próprio Windows (GDI) num atlas com os caracteres Latin-1 (32 a 255), que cobre todos os acentos do português (á, à, ã, â, ç, é, ê, í, ó, ô, õ, ú). O atlas vira textura com transparência.
  - Os textos são lidos em UTF-8.
  - API: `drawText`, `drawWrapped` (quebra de linha), `drawBox` (fundo semitransparente), `measure`, `wrap` e `lineHeight`, com cor e escala.
- **Caixa de mensagem** na parte de baixo da tela, com fundo escuro semitransparente e um filete amarelo no topo. Ela some sozinha depois de 6 segundos, com um esmaecimento no fim.
- **Mensagem de boas-vindas** ao abrir o jogo.
- **Linha discreta de dica de controles** no canto superior esquerdo.
- O tamanho do texto acompanha a altura da janela.
- `Renderer`: modo 2D (`begin2D`, `setMaterial2D`, `end2D`), com projeção ortográfica em pixels, sem teste de profundidade e com transparência.
- `Texture::createRGBA` (textura com canal alfa, sem mipmaps) e `MathUtils::orthographic`.

### Alterado
- As mensagens de interação (barco, placa, coqueiros) saíram do título da janela e passaram para a caixa na tela. O título agora é fixo.
- `basic.frag`: a textura também multiplica a transparência (as texturas RGB continuam opacas).
- `README.md`: pasta `src/ui` na organização do código.

### Removido
- A dica de controles e as mensagens no título da janela.

## 2026-10-07 — Roteiro de criação do jogo

**Pedido:** organizar os próximos passos do jogo num roteiro.

### Adicionado
- Roteiro de criação do jogo (documento do projeto) com as fases:
  - A: fundações;
  - B: capítulo 1;
  - C: troca de cena;
  - D: capítulos 2 a 5;
  - E: qualidade visual e som;
  - F: polimento.
- A próxima etapa indicada foi a A1 (texto na tela), feita na entrada acima.

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
- `build.bat`: o `-lwinmm` não tinha entrado de fato no arquivo, e o link falhava com `undefined reference to waveOutOpen` e outras funções do waveOut. Corrigido e validado rodando o próprio `build.bat`. O arquivo também voltou a ter quebras de linha do Windows (CRLF), garantidas pelo `.gitattributes`.
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
