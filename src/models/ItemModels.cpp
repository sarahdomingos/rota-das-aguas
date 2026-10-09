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

Mesh ribbon(const Vec3& color)
{
    // Laco com duas alcas e duas pontas
    Mesh mesh;
    mesh.addSphere({ -0.1f, 0.22f, 0.0f }, { 0.1f, 0.07f, 0.04f }, 8, 4, color);
    mesh.addSphere({ 0.1f, 0.22f, 0.0f }, { 0.1f, 0.07f, 0.04f }, 8, 4, color);
    mesh.addSphere({ 0.0f, 0.22f, 0.0f }, { 0.035f, 0.045f, 0.045f }, 6, 4, scale(color, 0.8f));
    mesh.addBox({ -0.05f, 0.11f, 0.0f }, { 0.04f, 0.2f, 0.015f }, color);
    mesh.addBox({ 0.05f, 0.11f, 0.0f }, { 0.04f, 0.2f, 0.015f }, color);
    return mesh;
}

}

Mesh build(const std::string& model, const Vec3& color)
{
    if (model == "coco") return coconut(color);
    if (model == "concha") return shell(color);
    if (model == "fita") return ribbon(color);

    Mesh box;
    box.addBox({ 0.0f, 0.12f, 0.0f }, { 0.24f, 0.24f, 0.24f }, color);
    return box;
}

}
