#include "ItemModels.h"

#include <cmath>

using namespace MathUtils;

namespace ItemModels {

namespace {

Mesh coconut(const Vec3& color)
{
    Mesh mesh;
    mesh.addSphere({ 0.0f, 0.17f, 0.0f }, { 0.16f, 0.17f, 0.16f }, 16, 10, color);
    mesh.addFrustum({ 0.0f, 0.32f, 0.0f }, { 0.0f, 0.38f, 0.0f }, 0.05f, 0.0f, 8, scale(color, 0.7f));
    return mesh;
}

Mesh shell(const Vec3& color)
{
    // Leque com gomos alternando de tom
    Mesh mesh;
    const int ribs = 9;
    Vec3 hinge = { 0.0f, 0.02f, 0.12f };

    for (int i = 0; i < ribs; ++i) {
        float a0 = PI * (0.1f + 0.8f * i / ribs);
        float a1 = PI * (0.1f + 0.8f * (i + 1) / ribs);
        Vec3 p0 = { std::cos(a0) * 0.2f, 0.03f, 0.12f - std::sin(a0) * 0.24f };
        Vec3 p1 = { std::cos(a1) * 0.2f, 0.03f, 0.12f - std::sin(a1) * 0.24f };
        Vec3 ridge = { (p0.x + p1.x) * 0.5f, 0.09f, (p0.z + p1.z) * 0.5f };
        Vec3 tone = (i % 2 == 0) ? color : scale(color, 0.85f);
        mesh.addTriangle(hinge, ridge, p0, tone);
        mesh.addTriangle(hinge, p1, ridge, tone);
    }
    return mesh;
}

Mesh bottle(const Vec3& color)
{
    // Garrafa deitada feita por revolucao (torno): fundo, corpo, ombro e
    // gargalo saem de um unico perfil, sem emendas visiveis. Tampinha a parte.
    Mesh mesh;
    const std::vector<Mesh::ProfilePoint> profile = {
        { 0.00f, 0.000f }, { 0.01f, 0.045f }, { 0.04f, 0.066f }, { 0.08f, 0.070f },
        { 0.58f, 0.070f }, { 0.66f, 0.064f }, { 0.74f, 0.047f }, { 0.80f, 0.033f },
        { 0.86f, 0.029f }, { 0.94f, 0.029f }, { 0.95f, 0.000f }
    };
    mesh.addLathe({ -0.16f, 0.07f, 0.0f }, { 0.24f, 0.07f, 0.0f }, profile, 16, color);

    const std::vector<Mesh::ProfilePoint> cap = {
        { 0.0f, 0.0f }, { 0.05f, 0.034f }, { 0.25f, 0.036f }, { 0.75f, 0.036f }, { 0.95f, 0.032f }, { 1.0f, 0.0f }
    };
    mesh.addLathe({ 0.205f, 0.07f, 0.0f }, { 0.24f, 0.07f, 0.0f }, cap, 12, { 0.9f, 0.9f, 0.9f });
    return mesh;
}

Mesh can(const Vec3& color)
{
    // Lata (um pouco inclinada) por revolucao: bordas arredondadas e aro prateado
    Mesh mesh;
    const std::vector<Mesh::ProfilePoint> body = {
        { 0.00f, 0.000f }, { 0.01f, 0.050f }, { 0.04f, 0.063f }, { 0.08f, 0.065f },
        { 0.90f, 0.065f }, { 0.96f, 0.058f }, { 1.00f, 0.000f }
    };
    mesh.addLathe({ 0.0f, 0.0f, 0.0f }, { 0.03f, 0.2f, 0.0f }, body, 16, color);

    const std::vector<Mesh::ProfilePoint> rim = {
        { 0.0f, 0.055f }, { 0.3f, 0.06f }, { 0.7f, 0.056f }, { 1.0f, 0.0f }
    };
    mesh.addLathe({ 0.027f, 0.18f, 0.0f }, { 0.032f, 0.215f, 0.0f }, rim, 16, { 0.8f, 0.8f, 0.82f });
    return mesh;
}

Mesh bag(const Vec3& color)
{
    // Sacola plastica embolada
    Mesh mesh;
    mesh.addSphere({ 0.0f, 0.1f, 0.0f }, { 0.16f, 0.1f, 0.13f }, 14, 9, color);
    mesh.addSphere({ 0.08f, 0.17f, 0.03f }, { 0.07f, 0.06f, 0.06f }, 10, 7, color);
    mesh.addFrustum({ -0.05f, 0.17f, 0.0f }, { -0.09f, 0.26f, 0.0f }, 0.025f, 0.015f, 8, color);
    return mesh;
}

}

Mesh build(const std::string& model, const Vec3& color)
{
    if (model == "coco") return coconut(color);
    if (model == "concha") return shell(color);
    if (model == "garrafa") return bottle(color);
    if (model == "lata") return can(color);
    if (model == "sacola") return bag(color);

    Mesh box;
    box.addRoundedBox({ 0.0f, 0.12f, 0.0f }, { 0.24f, 0.24f, 0.24f }, 0.04f, color);
    return box;
}

}
