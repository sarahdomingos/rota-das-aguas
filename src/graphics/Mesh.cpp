#include "Mesh.h"

#include <GLFW/glfw3.h>

#include <cmath>

using namespace MathUtils;

void Mesh::addVertex(const Vec3& position, const Vec3& normal, const Vec3& color, const TexCoord& uv)
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

    m_texCoords.push_back(uv.u);
    m_texCoords.push_back(uv.v);
}

// Vertices em sentido anti-horario; a normal sai da face (regra da mao direita).
void Mesh::addTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                       const TexCoord& ta, const TexCoord& tb, const TexCoord& tc,
                       const Vec3& color)
{
    Vec3 normal = normalize(cross(subtract(b, a), subtract(c, a)));

    addVertex(a, normal, color, ta);
    addVertex(b, normal, color, tb);
    addVertex(c, normal, color, tc);
}

void Mesh::addTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& color)
{
    addTriangle(a, b, c, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.5f, 1.0f }, color);
}

void Mesh::addQuad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& color,
                   float uSize, float vSize)
{
    TexCoord ta = { 0.0f, 0.0f };
    TexCoord tb = { uSize, 0.0f };
    TexCoord tc = { uSize, vSize };
    TexCoord td = { 0.0f, vSize };

    addTriangle(a, b, c, ta, tb, tc, color);
    addTriangle(a, c, d, ta, tc, td, color);
}

void Mesh::addBox(const Vec3& center, const Vec3& size, const Vec3& color)
{
    float x0 = center.x - size.x * 0.5f, x1 = center.x + size.x * 0.5f;
    float y0 = center.y - size.y * 0.5f, y1 = center.y + size.y * 0.5f;
    float z0 = center.z - size.z * 0.5f, z1 = center.z + size.z * 0.5f;

    // Frente (-Z), tras (+Z), esquerda (-X), direita (+X), topo (+Y), base (-Y)
    addQuad({ x1, y0, z0 }, { x0, y0, z0 }, { x0, y1, z0 }, { x1, y1, z0 }, color, size.x, size.y);
    addQuad({ x0, y0, z1 }, { x1, y0, z1 }, { x1, y1, z1 }, { x0, y1, z1 }, color, size.x, size.y);
    addQuad({ x0, y0, z0 }, { x0, y0, z1 }, { x0, y1, z1 }, { x0, y1, z0 }, color, size.z, size.y);
    addQuad({ x1, y0, z1 }, { x1, y0, z0 }, { x1, y1, z0 }, { x1, y1, z1 }, color, size.z, size.y);
    addQuad({ x0, y1, z1 }, { x1, y1, z1 }, { x1, y1, z0 }, { x0, y1, z0 }, color, size.x, size.z);
    addQuad({ x0, y0, z0 }, { x1, y0, z0 }, { x1, y0, z1 }, { x0, y0, z1 }, color, size.x, size.z);
}

void Mesh::addSmoothTriangle(const Vec3& a, const Vec3& b, const Vec3& c,
                             const Vec3& na, const Vec3& nb, const Vec3& nc,
                             const TexCoord& ta, const TexCoord& tb, const TexCoord& tc,
                             const Vec3& color)
{
    // Garante o sentido anti-horario visto de fora: se a face aponta contra as
    // normais dadas, troca a ordem de dois vertices.
    Vec3 face = cross(subtract(b, a), subtract(c, a));
    if (dot(face, add(add(na, nb), nc)) < 0.0f) {
        addVertex(a, na, color, ta);
        addVertex(c, nc, color, tc);
        addVertex(b, nb, color, tb);
        return;
    }

    addVertex(a, na, color, ta);
    addVertex(b, nb, color, tb);
    addVertex(c, nc, color, tc);
}

void Mesh::addFrustum(const Vec3& p0, const Vec3& p1, float r0, float r1, int segments,
                      const Vec3& color, bool caps)
{
    Vec3 axis = normalize(subtract(p1, p0));
    float height = length(subtract(p1, p0));

    // Base ortonormal perpendicular ao eixo
    Vec3 helper = std::fabs(axis.y) < 0.99f ? Vec3{ 0.0f, 1.0f, 0.0f } : Vec3{ 1.0f, 0.0f, 0.0f };
    Vec3 u = normalize(cross(helper, axis));
    Vec3 v = cross(axis, u);

    float circumference = 2.0f * PI * (r0 + r1) * 0.5f;

    // Normal suave da lateral: aponta para fora (raio) e inclina conforme o cone
    auto sideNormal = [&](const Vec3& d) {
        return normalize(add(scale(d, height), scale(axis, r0 - r1)));
    };

    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        Vec3 d0 = add(scale(u, std::cos(a0)), scale(v, std::sin(a0)));
        Vec3 d1 = add(scale(u, std::cos(a1)), scale(v, std::sin(a1)));

        Vec3 b0 = add(p0, scale(d0, r0));
        Vec3 b1 = add(p0, scale(d1, r0));
        Vec3 t0 = add(p1, scale(d0, r1));
        Vec3 t1 = add(p1, scale(d1, r1));

        Vec3 n0 = sideNormal(d0);
        Vec3 n1 = sideNormal(d1);

        float uu0 = circumference * i / segments;
        float uu1 = circumference * (i + 1) / segments;

        addSmoothTriangle(b0, b1, t1, n0, n1, n1, { uu0, 0.0f }, { uu1, 0.0f }, { uu1, height }, color);
        addSmoothTriangle(b0, t1, t0, n0, n1, n0, { uu0, 0.0f }, { uu1, height }, { uu0, height }, color);

        if (caps) {
            if (r1 > 0.0001f) addTriangle(p1, t0, t1, color);
            if (r0 > 0.0001f) addTriangle(p0, b1, b0, color);
        }
    }
}

void Mesh::addLathe(const Vec3& p0, const Vec3& p1, const std::vector<ProfilePoint>& profile,
                    int segments, const Vec3& color)
{
    if (profile.size() < 2) {
        return;
    }

    Vec3 axis = normalize(subtract(p1, p0));
    float height = length(subtract(p1, p0));

    Vec3 helper = std::fabs(axis.y) < 0.99f ? Vec3{ 0.0f, 1.0f, 0.0f } : Vec3{ 1.0f, 0.0f, 0.0f };
    Vec3 u = normalize(cross(helper, axis));
    Vec3 v = cross(axis, u);

    const int count = static_cast<int>(profile.size());

    // Normal do perfil em cada ponto (no plano eixo x raio), pela media dos
    // vizinhos: e isso que deixa as passagens corpo -> ombro -> gargalo lisas.
    std::vector<float> normalAxial(count);
    std::vector<float> normalRadial(count);
    for (int i = 0; i < count; ++i) {
        const ProfilePoint& before = profile[i > 0 ? i - 1 : i];
        const ProfilePoint& after = profile[i < count - 1 ? i + 1 : i];
        float dt = (after.t - before.t) * height;
        float dr = after.radius - before.radius;

        float axial = -dr;
        float radial = dt;

        // Pontas fechadas (raio zero): a normal aponta ao longo do eixo
        if (profile[i].radius < 0.0001f) {
            axial = (i == 0) ? -1.0f : 1.0f;
            radial = 0.0f;
        }

        float len = std::sqrt(axial * axial + radial * radial);
        normalAxial[i] = len > 0.0f ? axial / len : 0.0f;
        normalRadial[i] = len > 0.0f ? radial / len : 1.0f;
    }

    auto point = [&](int ring, int slice, Vec3& position, Vec3& normal) {
        float angle = 2.0f * PI * slice / segments;
        Vec3 d = add(scale(u, std::cos(angle)), scale(v, std::sin(angle)));
        Vec3 center = add(p0, scale(axis, profile[ring].t * height));
        position = add(center, scale(d, profile[ring].radius));
        normal = normalize(add(scale(d, normalRadial[ring]), scale(axis, normalAxial[ring])));
    };

    for (int ring = 0; ring < count - 1; ++ring) {
        for (int slice = 0; slice < segments; ++slice) {
            Vec3 a, b, c, d;
            Vec3 na, nb, nc, nd;
            point(ring, slice, a, na);
            point(ring, slice + 1, b, nb);
            point(ring + 1, slice + 1, c, nc);
            point(ring + 1, slice, d, nd);

            float u0 = static_cast<float>(slice) / segments;
            float u1 = static_cast<float>(slice + 1) / segments;
            TexCoord ta = { u0, profile[ring].t }, tb = { u1, profile[ring].t };
            TexCoord tc = { u1, profile[ring + 1].t }, td = { u0, profile[ring + 1].t };

            if (profile[ring].radius > 0.0001f) {
                addSmoothTriangle(a, b, c, na, nb, nc, ta, tb, tc, color);
            }
            if (profile[ring + 1].radius > 0.0001f) {
                addSmoothTriangle(a, c, d, na, nc, nd, ta, tc, td, color);
            }
        }
    }
}

void Mesh::addRoundedBox(const Vec3& center, const Vec3& size, float radius, const Vec3& color,
                         int bevelSteps)
{
    float half[3] = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
    float r = std::fmin(radius, std::fmin(half[0], std::fmin(half[1], half[2])));
    if (bevelSteps < 1) bevelSteps = 1;

    // Coordenadas da grade em um eixo: mais pontos perto das bordas (onde dobra)
    auto samples = [&](float h) {
        std::vector<float> list;
        for (int k = 0; k <= bevelSteps; ++k) list.push_back(-h + r * k / bevelSteps);
        for (int k = 0; k <= bevelSteps; ++k) {
            float value = (h - r) + r * k / bevelSteps;
            if (k == 0 && h - r < 0.00001f) continue; // evita ponto repetido
            list.push_back(value);
        }
        return list;
    };

    // Ponto da face -> superficie arredondada: parte "interna" (caixa encolhida
    // pelo raio) + raio na direcao do ponto. A normal e essa mesma direcao.
    auto roundPoint = [&](const float p[3], Vec3& position, Vec3& normal) {
        float inner[3];
        float diff[3];
        for (int k = 0; k < 3; ++k) {
            inner[k] = clamp(p[k], -(half[k] - r), half[k] - r);
            diff[k] = p[k] - inner[k];
        }
        normal = normalize({ diff[0], diff[1], diff[2] });
        position = { center.x + inner[0] + normal.x * r,
                     center.y + inner[1] + normal.y * r,
                     center.z + inner[2] + normal.z * r };
    };

    for (int axis = 0; axis < 3; ++axis) {
        int ua = (axis + 1) % 3;
        int va = (axis + 2) % 3;
        std::vector<float> su = samples(half[ua]);
        std::vector<float> sv = samples(half[va]);

        for (int side = -1; side <= 1; side += 2) {
            for (size_t i = 0; i + 1 < su.size(); ++i) {
                for (size_t j = 0; j + 1 < sv.size(); ++j) {
                    Vec3 pos[4];
                    Vec3 nor[4];
                    TexCoord uv[4];
                    const size_t ii[4] = { i, i + 1, i + 1, i };
                    const size_t jj[4] = { j, j, j + 1, j + 1 };

                    for (int k = 0; k < 4; ++k) {
                        float p[3];
                        p[axis] = side * half[axis];
                        p[ua] = su[ii[k]];
                        p[va] = sv[jj[k]];
                        roundPoint(p, pos[k], nor[k]);
                        uv[k] = { p[ua], p[va] };
                    }

                    addSmoothTriangle(pos[0], pos[1], pos[2], nor[0], nor[1], nor[2], uv[0], uv[1], uv[2], color);
                    addSmoothTriangle(pos[0], pos[2], pos[3], nor[0], nor[2], nor[3], uv[0], uv[2], uv[3], color);
                }
            }
        }
    }
}

void Mesh::addSphere(const Vec3& center, const Vec3& radii, int slices, int stacks, const Vec3& color)
{
    auto point = [&](int slice, int stack) {
        float theta = PI * stack / stacks;          // 0 no topo, PI na base
        float phi = 2.0f * PI * slice / slices;
        return Vec3{
            center.x + radii.x * std::sin(theta) * std::cos(phi),
            center.y + radii.y * std::cos(theta),
            center.z + radii.z * std::sin(theta) * std::sin(phi)
        };
    };

    auto uv = [&](int slice, int stack) {
        return TexCoord{ static_cast<float>(slice) / slices, 1.0f - static_cast<float>(stack) / stacks };
    };

    // Normal suave do elipsoide: gradiente da superficie em cada vertice
    auto normalAt = [&](const Vec3& p) {
        Vec3 offset = subtract(p, center);
        return normalize({ offset.x / (radii.x * radii.x), offset.y / (radii.y * radii.y),
                           offset.z / (radii.z * radii.z) });
    };

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            Vec3 a = point(slice, stack);
            Vec3 b = point(slice + 1, stack);
            Vec3 c = point(slice + 1, stack + 1);
            Vec3 d = point(slice, stack + 1);

            if (stack != 0) {
                addSmoothTriangle(a, b, d, normalAt(a), normalAt(b), normalAt(d),
                                  uv(slice, stack), uv(slice + 1, stack), uv(slice, stack + 1), color);
            }
            if (stack != stacks - 1) {
                addSmoothTriangle(b, c, d, normalAt(b), normalAt(c), normalAt(d),
                                  uv(slice + 1, stack), uv(slice + 1, stack + 1), uv(slice, stack + 1), color);
            }
        }
    }
}

void Mesh::append(const Mesh& other, const float matrix[16], const Vec3& tint)
{
    float normals[16];
    normalMatrix(matrix, normals);

    for (std::size_t i = 0; i < other.getVertexCount(); ++i) {
        Vec3 p = { other.m_positions[i * 3], other.m_positions[i * 3 + 1], other.m_positions[i * 3 + 2] };
        Vec3 n = { other.m_normals[i * 3], other.m_normals[i * 3 + 1], other.m_normals[i * 3 + 2] };
        Vec3 c = { other.m_colors[i * 3] * tint.x, other.m_colors[i * 3 + 1] * tint.y, other.m_colors[i * 3 + 2] * tint.z };
        TexCoord t = { other.m_texCoords[i * 2], other.m_texCoords[i * 2 + 1] };

        addVertex(transformPoint(matrix, p), normalize(transformDirection(normals, n)), c, t);
    }
}

Mesh Mesh::createGrid(float minX, float minZ, float maxX, float maxZ, int cellsX, int cellsZ,
                      const std::function<float(float, float)>& height,
                      const std::function<Vec3(float, float, float)>& color)
{
    Mesh mesh;

    float stepX = (maxX - minX) / cellsX;
    float stepZ = (maxZ - minZ) / cellsZ;

    auto vertex = [&](int ix, int iz) {
        float x = minX + ix * stepX;
        float z = minZ + iz * stepZ;
        return Vec3{ x, height(x, z), z };
    };

    for (int iz = 0; iz < cellsZ; ++iz) {
        for (int ix = 0; ix < cellsX; ++ix) {
            Vec3 a = vertex(ix, iz + 1);
            Vec3 b = vertex(ix + 1, iz + 1);
            Vec3 c = vertex(ix + 1, iz);
            Vec3 d = vertex(ix, iz);

            // Alterna a diagonal para o relevo nao ficar "listrado"
            bool flip = (ix + iz) % 2 == 0;

            Vec3 t1[3] = { a, b, flip ? c : d };
            Vec3 t2[3] = { flip ? a : b, c, d };

            for (int k = 0; k < 2; ++k) {
                const Vec3* t = (k == 0) ? t1 : t2;
                Vec3 middle = scale(add(add(t[0], t[1]), t[2]), 1.0f / 3.0f);
                Vec3 col = color(middle.x, middle.y, middle.z);

                mesh.addTriangle(t[0], t[1], t[2],
                                 { t[0].x, t[0].z }, { t[1].x, t[1].z }, { t[2].x, t[2].z }, col);
            }
        }
    }

    return mesh;
}

Mesh Mesh::createWaterGrid(float size, int cells)
{
    Mesh mesh;

    const Vec3 white = { 1.0f, 1.0f, 1.0f };
    const Vec3 up = { 0.0f, 1.0f, 0.0f };

    float start = -0.5f * size;
    float step = size / cells;

    for (int row = 0; row < cells; ++row) {
        for (int column = 0; column < cells; ++column) {
            float x0 = start + column * step;
            float z0 = start + row * step;
            float x1 = x0 + step;
            float z1 = z0 + step;

            Vec3 a = { x0, 0.0f, z1 }, b = { x1, 0.0f, z1 }, c = { x1, 0.0f, z0 }, d = { x0, 0.0f, z0 };

            mesh.addVertex(a, up, white, { x0, z1 });
            mesh.addVertex(b, up, white, { x1, z1 });
            mesh.addVertex(c, up, white, { x1, z0 });
            mesh.addVertex(a, up, white, { x0, z1 });
            mesh.addVertex(c, up, white, { x1, z0 });
            mesh.addVertex(d, up, white, { x0, z0 });
        }
    }

    return mesh;
}

Mesh Mesh::createDisc(int segments)
{
    Mesh mesh;

    const Vec3 white = { 1.0f, 1.0f, 1.0f };

    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        mesh.addTriangle({ 0.0f, 0.0f, 0.0f },
                         { std::cos(a0), 0.0f, -std::sin(a0) },
                         { std::cos(a1), 0.0f, -std::sin(a1) }, white);
    }

    return mesh;
}

Mesh Mesh::createSkyDome(const Vec3& horizon, const Vec3& zenith)
{
    Mesh mesh;

    const int slices = 24;
    const int stacks = 12;

    auto point = [&](int slice, int stack) {
        float theta = PI * stack / stacks;
        float phi = 2.0f * PI * slice / slices;
        return Vec3{ std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi) };
    };

    auto skyColor = [&](const Vec3& p) {
        float t = std::pow(clamp(p.y, 0.0f, 1.0f), 0.6f);
        return lerp(horizon, zenith, t);
    };

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            Vec3 p[4] = { point(slice, stack), point(slice + 1, stack), point(slice + 1, stack + 1), point(slice, stack + 1) };
            int order[6] = { 0, 1, 2, 0, 2, 3 };

            for (int k = 0; k < 6; ++k) {
                const Vec3& v = p[order[k]];
                mesh.addVertex(v, scale(v, -1.0f), skyColor(v), { 0.0f, 0.0f });
            }
        }
    }

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
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, m_positions.data());
    glNormalPointer(GL_FLOAT, 0, m_normals.data());
    glColorPointer(3, GL_FLOAT, 0, m_colors.data());
    glTexCoordPointer(2, GL_FLOAT, 0, m_texCoords.data());

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(getVertexCount()));

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

std::size_t Mesh::getVertexCount() const
{
    return m_positions.size() / 3;
}

bool Mesh::empty() const
{
    return m_positions.empty();
}
