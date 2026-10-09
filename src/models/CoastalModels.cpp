#include "CoastalModels.h"

#include <cmath>
#include <cstdint>

using namespace MathUtils;

namespace CoastalModels {

namespace {

float random01(int a, int b)
{
    uint32_t h = static_cast<uint32_t>(a) * 73856093u ^ static_cast<uint32_t>(b) * 19349663u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFF) / 65535.0f;
}

}

Mesh palmTrunk(float height, float lean, Vec3& top)
{
    Mesh mesh;

    const int segments = 8;
    const Vec3 barkA = { 0.62f, 0.50f, 0.38f };
    const Vec3 barkB = { 0.52f, 0.41f, 0.31f };

    auto point = [&](float t) {
        return Vec3{ lean * t * t, height * t, 0.0f };
    };

    for (int i = 0; i < segments; ++i) {
        float t0 = static_cast<float>(i) / segments;
        float t1 = static_cast<float>(i + 1) / segments;

        // Base mais grossa e aneis alternando de tom
        float r0 = lerp(0.24f, 0.13f, t0);
        float r1 = lerp(0.24f, 0.13f, t1);
        if (i == 0) r0 = 0.3f;

        mesh.addFrustum(point(t0), point(t1), r0, r1 * 0.92f, 7, (i % 2 == 0) ? barkA : barkB);
    }

    top = point(1.0f);

    // "Coroa" escura e cocos
    mesh.addSphere(top, { 0.2f, 0.16f, 0.2f }, 7, 5, { 0.30f, 0.33f, 0.14f });

    const Vec3 coconutColors[2] = { { 0.32f, 0.42f, 0.14f }, { 0.42f, 0.30f, 0.14f } };
    for (int i = 0; i < 4; ++i) {
        float angle = 2.0f * PI * i / 4.0f + 0.4f;
        Vec3 position = { top.x + std::cos(angle) * 0.17f, top.y - 0.2f, top.z + std::sin(angle) * 0.17f };
        mesh.addSphere(position, { 0.115f, 0.13f, 0.115f }, 7, 5, coconutColors[i % 2]);
    }

    return mesh;
}

Mesh palmFrond(float length)
{
    Mesh mesh;

    const int segments = 12;
    const Vec3 baseColor = { 0.20f, 0.46f, 0.15f };
    const Vec3 tipColor = { 0.45f, 0.62f, 0.20f };

    // Nervura central: sobe um pouco e depois cai (parabola)
    auto spine = [&](float u) {
        return Vec3{ length * u, 0.55f * u - 1.45f * u * u, 0.0f };
    };

    for (int i = 0; i < segments; ++i) {
        float u0 = static_cast<float>(i) / segments;
        float u1 = static_cast<float>(i + 1) / segments;
        float um = (u0 + u1) * 0.5f;

        Vec3 a = spine(u0);
        Vec3 b = spine(u1);
        Vec3 color = lerp(baseColor, tipColor, um);

        // Foliolos dos dois lados, cada vez mais estreitos na ponta
        float width = 0.55f * std::sin(PI * std::fmin(um * 1.1f, 1.0f)) + 0.06f;

        for (int side = -1; side <= 1; side += 2) {
            Vec3 tip = add(spine(um + 0.06f), { 0.12f, -0.22f * width, side * width });
            mesh.addTriangle(a, b, tip, color);
        }

        // Nervura com espessura (visivel de lado)
        mesh.addTriangle(a, b, add(b, { 0.0f, -0.03f, 0.0f }), lerp(color, { 0.5f, 0.45f, 0.2f }, 0.5f));
    }

    return mesh;
}

Mesh rock(int seed, float size)
{
    Mesh mesh;

    const int slices = 8;
    const int stacks = 6;

    // O raio depende so da direcao, entao vertices compartilhados batem.
    auto radiusAt = [&](int slice, int stack) {
        if (stack == 0 || stack == stacks) {
            return 0.8f;
        }
        return 0.75f + 0.45f * random01(seed * 31 + slice % slices, stack);
    };

    auto point = [&](int slice, int stack) {
        float theta = PI * stack / stacks;
        float phi = 2.0f * PI * slice / slices;
        float r = radiusAt(slice, stack) * size;
        return Vec3{ r * std::sin(theta) * std::cos(phi), r * std::cos(theta) * 0.65f + size * 0.35f,
                     r * std::sin(theta) * std::sin(phi) };
    };

    auto colorAt = [&](const Vec3& p) {
        float shade = 0.85f + 0.2f * random01(seed, static_cast<int>(p.x * 50 + p.z * 70));
        Vec3 dry = { 0.56f * shade, 0.53f * shade, 0.49f * shade };
        Vec3 algae = { 0.30f, 0.36f, 0.27f };
        return lerp(algae, dry, smoothstep(0.0f, size * 0.5f, p.y));
    };

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            Vec3 a = point(slice, stack);
            Vec3 b = point(slice + 1, stack);
            Vec3 c = point(slice + 1, stack + 1);
            Vec3 d = point(slice, stack + 1);

            if (stack != 0) {
                mesh.addTriangle(a, b, d, colorAt(scale(add(add(a, b), d), 1.0f / 3.0f)));
            }
            if (stack != stacks - 1) {
                mesh.addTriangle(b, c, d, colorAt(scale(add(add(b, c), d), 1.0f / 3.0f)));
            }
        }
    }

    return mesh;
}

Mesh cloud(int seed)
{
    Mesh mesh;

    int puffs = 4 + static_cast<int>(random01(seed, 1) * 3.0f);

    for (int i = 0; i < puffs; ++i) {
        float x = (i - puffs * 0.5f) * 2.6f + random01(seed, i * 3) * 1.5f;
        float z = (random01(seed, i * 3 + 1) - 0.5f) * 3.0f;
        float r = 2.2f + random01(seed, i * 3 + 2) * 1.8f;
        float shade = 0.95f + 0.05f * random01(seed, i);

        mesh.addSphere({ x, r * 0.25f, z }, { r * 1.2f, r * 0.7f, r }, 8, 5, { shade, shade, shade });
    }

    return mesh;
}

Mesh beachUmbrella(const Vec3& colorA, const Vec3& colorB)
{
    Mesh mesh;

    mesh.addFrustum({ 0.0f, -0.3f, 0.0f }, { 0.0f, 2.25f, 0.0f }, 0.035f, 0.03f, 6, { 0.9f, 0.9f, 0.88f });

    // Lona: cone de gomos alternando as cores
    const int segments = 12;
    const float radius = 1.35f;
    const Vec3 apex = { 0.0f, 2.45f, 0.0f };

    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        Vec3 p0 = { std::cos(a0) * radius, 1.95f, -std::sin(a0) * radius };
        Vec3 p1 = { std::cos(a1) * radius, 1.95f, -std::sin(a1) * radius };
        const Vec3& color = (i % 2 == 0) ? colorA : colorB;

        mesh.addTriangle(apex, p0, p1, color);

        // Babado da borda
        mesh.addQuad(p1, p0, add(p0, { 0.0f, -0.12f, 0.0f }), add(p1, { 0.0f, -0.12f, 0.0f }), color);
    }

    mesh.addSphere(apex, { 0.05f, 0.05f, 0.05f }, 5, 3, { 0.9f, 0.9f, 0.88f });

    return mesh;
}

}
