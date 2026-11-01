#ifndef MESH_H
#define MESH_H

// Malha de triangulos com posicao, normal, cor e coordenada de textura por
// vertice, desenhada com vertex arrays (glDrawArrays).
//
// As funcoes add* sao os "tijolos" usados pelos geradores de modelos
// (src/models): cada modelo low-poly e montado juntando primitivas.
// Caixas e triangulos soltos usam normal por face (visual facetado low-poly);
// esferas, cilindros, superficies de revolucao (lathe) e caixas arredondadas
// usam normal suave por vertice, entao a luz nao mostra as "emendas".

#include "MathUtils.h"

#include <cstddef>
#include <functional>
#include <vector>

struct TexCoord {
    float u;
    float v;
};

class Mesh {
public:
    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& color);
    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                     const TexCoord& ta, const TexCoord& tb, const TexCoord& tc,
                     const Vec3& color);

    // Quadrilatero a-b-c-d em sentido anti-horario (visto de fora).
    void addQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& color,
                 float uSize = 1.0f, float vSize = 1.0f);

    // Caixa alinhada aos eixos; a textura repete a cada 1 unidade de mundo.
    void addBox(const Vec3& center, const Vec3& size, const Vec3& color);

    // Cilindro (ou cone) entre os pontos p0 e p1, com raios r0 e r1.
    void addFrustum(const Vec3& p0, const Vec3& p1, float r0, float r1, int segments,
                    const Vec3& color, bool caps = true);

    // Caixa com cantos arredondados (raio radius): cada face e uma grade que
    // "dobra" nas bordas, com normal suave. Fica menos pontuda que a addBox.
    void addRoundedBox(const Vec3& center, const Vec3& size, float radius, const Vec3& color,
                       int bevelSteps = 3);

    // Superficie de revolucao (torno): o perfil (t, raio) gira em torno do eixo
    // p0 -> p1 (t = 0 em p0, t = 1 em p1). Uma unica peca lisa, sem emendas
    // entre partes (ex.: garrafa com corpo, ombro e gargalo).
    struct ProfilePoint { float t; float radius; };
    void addLathe(const Vec3& p0, const Vec3& p1, const std::vector<ProfilePoint>& profile,
                  int segments, const Vec3& color);

    // Triangulo com uma normal por vertice (sombreamento suave).
    void addSmoothTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                           const Vec3& na, const Vec3& nb, const Vec3& nc,
                           const TexCoord& ta, const TexCoord& tb, const TexCoord& tc,
                           const Vec3& color);

    // Esfera low-poly (elipsoide se os raios forem diferentes).
    void addSphere(const Vec3& center, const Vec3& radii, int slices, int stacks, const Vec3& color);

    // Copia outra malha aplicando uma matriz e multiplicando as cores por tint.
    void append(const Mesh& other, const float matrix[16], const Vec3& tint = { 1.0f, 1.0f, 1.0f });

    // Grade no plano XZ com altura e cor dadas por funcoes (terreno).
    static Mesh createGrid(float minX, float minZ, float maxX, float maxZ, int cellsX, int cellsZ,
                           const std::function<float(float, float)>& height,
                           const std::function<Vec3(float, float, float)>& color);

    // Plano de agua subdividido (y = 0); as ondas sao feitas no vertex shader.
    static Mesh createWaterGrid(float size, int cells);

    // Disco de raio 1 no plano XZ (sombra simples).
    static Mesh createDisc(int segments);

    // Cupula do ceu com degrade do horizonte ate o zenite.
    static Mesh createSkyDome(const Vec3& horizon, const Vec3& zenith);

    void draw() const;

    std::size_t getVertexCount() const;
    bool empty() const;

private:
    std::vector<float> m_positions;
    std::vector<float> m_normals;
    std::vector<float> m_colors;
    std::vector<float> m_texCoords;

    void addVertex(const Vec3& position, const Vec3& normal, const Vec3& color, const TexCoord& uv);
};

#endif
