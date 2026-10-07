#include "Mesh.h"

#include <GLFW/glfw3.h>

void Mesh::addVertex(const Vec3& position, const Vec3& normal, const Vec3& color)
{
    m_positions.push_back(position.x);
    m_positions.push_back(position.y);
    m_positions.push_back(position.z);

    m_normals.push_back(normal.x);
    m_normals.push_back(normal.y);
    m_normals.push_back(normal.z);

    m_colors.push_back(color.x);
    m_colors.push_back(color.y);
    m_colors.push_back(color.z);
}

// Vertices em sentido anti-horario; a normal sai da face (regra da mao direita).
void Mesh::addTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& color)
{
    Vec3 normal = MathUtils::normalize(
        MathUtils::cross(MathUtils::subtract(b, a), MathUtils::subtract(c, a))
    );

    addVertex(a, normal, color);
    addVertex(b, normal, color);
    addVertex(c, normal, color);
}

void Mesh::addQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& color)
{
    addTriangle(a, b, c, color);
    addTriangle(a, c, d, color);
}

Mesh Mesh::createCube()
{
    Mesh mesh;

    const float h = 0.5f;
    const Vec3 white = { 1.0f, 1.0f, 1.0f };

    // Frente (-Z), tras (+Z), esquerda (-X), direita (+X), topo (+Y), base (-Y)
    mesh.addQuad({  h, -h, -h }, { -h, -h, -h }, { -h,  h, -h }, {  h,  h, -h }, white);
    mesh.addQuad({ -h, -h,  h }, {  h, -h,  h }, {  h,  h,  h }, { -h,  h,  h }, white);
    mesh.addQuad({ -h, -h, -h }, { -h, -h,  h }, { -h,  h,  h }, { -h,  h, -h }, white);
    mesh.addQuad({  h, -h,  h }, {  h, -h, -h }, {  h,  h, -h }, {  h,  h,  h }, white);
    mesh.addQuad({ -h,  h,  h }, {  h,  h,  h }, {  h,  h, -h }, { -h,  h, -h }, white);
    mesh.addQuad({ -h, -h, -h }, {  h, -h, -h }, {  h, -h,  h }, { -h, -h,  h }, white);

    return mesh;
}

Mesh Mesh::createWaterGrid(int tilesPerSide, float tileSize)
{
    Mesh mesh;

    const Vec3 lightBlue = { 0.18f, 0.55f, 0.78f };
    const Vec3 darkBlue = { 0.12f, 0.45f, 0.70f };

    float start = -0.5f * tilesPerSide * tileSize;

    for (int row = 0; row < tilesPerSide; ++row) {
        for (int column = 0; column < tilesPerSide; ++column) {
            float x0 = start + column * tileSize;
            float z0 = start + row * tileSize;
            float x1 = x0 + tileSize;
            float z1 = z0 + tileSize;

            const Vec3& color = ((row + column) % 2 == 0) ? lightBlue : darkBlue;

            mesh.addQuad({ x0, 0.0f, z1 }, { x1, 0.0f, z1 }, { x1, 0.0f, z0 }, { x0, 0.0f, z0 }, color);
        }
    }

    return mesh;
}

Mesh Mesh::createBoatHull()
{
    Mesh mesh;

    const Vec3 wood = { 1.0f, 1.0f, 1.0f };

    // Proa em -Z e popa em +Z; borda superior em y = 0.25, quilha em y = -0.25.
    const Vec3 bow = { 0.0f, 0.25f, -1.0f };
    const Vec3 stern = { 0.0f, 0.25f, 1.0f };
    const Vec3 leftTop = { -0.35f, 0.25f, 0.0f };
    const Vec3 rightTop = { 0.35f, 0.25f, 0.0f };
    const Vec3 keelFront = { 0.0f, -0.25f, -0.6f };
    const Vec3 keelBack = { 0.0f, -0.25f, 0.6f };

    // Laterais
    mesh.addQuad(bow, keelFront, keelBack, leftTop, wood);
    mesh.addTriangle(leftTop, keelBack, stern, wood);
    mesh.addQuad(bow, rightTop, keelBack, keelFront, wood);
    mesh.addTriangle(rightTop, stern, keelBack, wood);

    // Tampa (convés)
    mesh.addQuad(bow, leftTop, stern, rightTop, wood);

    return mesh;
}

void Mesh::draw() const
{
    if (m_positions.empty()) {
        return;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, m_positions.data());
    glNormalPointer(GL_FLOAT, 0, m_normals.data());
    glColorPointer(3, GL_FLOAT, 0, m_colors.data());

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(getVertexCount()));

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

std::size_t Mesh::getVertexCount() const
{
    return m_positions.size() / 3;
}
