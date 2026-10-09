#ifndef MESH_H
#define MESH_H

// Malha de triangulos com posicao, normal, cor e coordenada de textura por
// vertice, desenhada com vertex arrays (glDrawArrays).
//
// As funcoes add* sao os "tijolos" usados pelos geradores de modelos
// (src/models): cada modelo low-poly e montado juntando primitivas.
// Os triangulos usam normal por face, o que da o visual facetado low-poly.

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
