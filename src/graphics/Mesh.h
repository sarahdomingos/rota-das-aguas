#ifndef MESH_H
#define MESH_H

// Renderizacao de primitivas: lista de triangulos com posicao, normal e cor
// por vertice, desenhada com vertex arrays (glDrawArrays).

#include "MathUtils.h"

#include <cstddef>
#include <vector>

class Mesh {
public:
    // Cubo unitario (de -0.5 a 0.5) branco; a cor final vem do "tint" do Renderer.
    static Mesh createCube();

    // Plano de agua no eixo XZ (y = 0) quadriculado em dois tons de azul,
    // para dar nocao de movimento quando a camera anda.
    static Mesh createWaterGrid(int tilesPerSide, float tileSize);

    // Casco simples de canoa, usado se o modelo .obj do barco nao carregar.
    static Mesh createBoatHull();

    void draw() const;

    std::size_t getVertexCount() const;

private:
    std::vector<float> m_positions;
    std::vector<float> m_normals;
    std::vector<float> m_colors;

    void addVertex(const Vec3& position, const Vec3& normal, const Vec3& color);
    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& color);
    void addQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& color);
};

#endif
