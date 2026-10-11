#include "ItemModels.h"

#include <cmath>

using namespace MathUtils;

namespace ItemModels {

namespace {

Mesh coconut(const Vec3& color)
{
    Mesh mesh;
    mesh.addSphere({ 0.0f, 0.17f, 0.0f }, { 0.16f, 0.17f, 0.16f }, 9, 6, color);
    mesh.addFrustum({ 0.0f, 0.32f, 0.0f }, { 0.0f, 0.38f, 0.0f }, 0.05f, 0.0f, 6, scale(color, 0.7f));
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
    // Garrafa deitada: corpo, gargalo e tampinha
    Mesh mesh;
    mesh.addFrustum({ -0.16f, 0.07f, 0.0f }, { 0.08f, 0.07f, 0.0f }, 0.07f, 0.07f, 8, color);
    mesh.addFrustum({ 0.08f, 0.07f, 0.0f }, { 0.16f, 0.07f, 0.0f }, 0.07f, 0.03f, 8, color);
    mesh.addFrustum({ 0.16f, 0.07f, 0.0f }, { 0.21f, 0.07f, 0.0f }, 0.03f, 0.03f, 6, color);
    mesh.addFrustum({ 0.21f, 0.07f, 0.0f }, { 0.24f, 0.07f, 0.0f }, 0.035f, 0.035f, 6, { 0.9f, 0.9f, 0.9f });
    return mesh;
}

Mesh can(const Vec3& color)
{
    // Lata amassada (um pouco inclinada) com borda prateada
    Mesh mesh;
    mesh.addFrustum({ 0.0f, 0.0f, 0.0f }, { 0.03f, 0.2f, 0.0f }, 0.065f, 0.065f, 9, color);
    mesh.addFrustum({ 0.03f, 0.2f, 0.0f }, { 0.032f, 0.22f, 0.0f }, 0.06f, 0.05f, 9, { 0.8f, 0.8f, 0.82f });
    return mesh;
}

Mesh bag(const Vec3& color)
{
    // Sacola plastica embolada
    Mesh mesh;
    mesh.addSphere({ 0.0f, 0.1f, 0.0f }, { 0.16f, 0.1f, 0.13f }, 7, 5, color);
    mesh.addSphere({ 0.08f, 0.17f, 0.03f }, { 0.07f, 0.06f, 0.06f }, 6, 4, color);
    mesh.addFrustum({ -0.05f, 0.17f, 0.0f }, { -0.09f, 0.26f, 0.0f }, 0.025f, 0.015f, 5, color);
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
    box.addBox({ 0.0f, 0.12f, 0.0f }, { 0.24f, 0.24f, 0.24f }, color);
    return box;
}

}
