#include "UrbanModels.h"

#include <cmath>
#include <cstdint>

using namespace MathUtils;

namespace UrbanModels {

namespace {

float random01(int a, int b)
{
    uint32_t h = static_cast<uint32_t>(a) * 2654435761u ^ static_cast<uint32_t>(b) * 40503u;
    h = (h ^ (h >> 15)) * 2246822519u;
    h ^= h >> 13;
    return (h & 0xFFFF) / 65535.0f;
}

Vec3 darker(const Vec3& c, float k)
{
    return { c.x * k, c.y * k, c.z * k };
}

// Janela com moldura, venezianas e peitoril na parede de frente (-Z) em z = wallZ.
void addFrontWindow(Mesh& mesh, float x, float y, float wallZ, const HouseDescription& house)
{
    mesh.addBox({ x, y, wallZ - 0.025f }, { 1.05f, 1.35f, 0.05f }, house.trimColor);
    mesh.addBox({ x - 0.22f, y, wallZ - 0.06f }, { 0.4f, 1.15f, 0.04f }, house.doorColor);
    mesh.addBox({ x + 0.22f, y, wallZ - 0.06f }, { 0.4f, 1.15f, 0.04f }, house.doorColor);
    // Ripas da veneziana
    for (int i = 0; i < 5; ++i) {
        float yy = y - 0.45f + i * 0.22f;
        mesh.addBox({ x, yy, wallZ - 0.085f }, { 0.82f, 0.03f, 0.015f }, darker(house.doorColor, 0.8f));
    }
    mesh.addBox({ x, y - 0.72f, wallZ - 0.08f }, { 1.15f, 0.08f, 0.16f }, house.trimColor);
}

void addSideWindow(Mesh& mesh, float x, float y, float z, float side, const HouseDescription& house)
{
    mesh.addBox({ x + side * 0.025f, y, z }, { 0.05f, 1.2f, 0.95f }, house.trimColor);
    mesh.addBox({ x + side * 0.06f, y, z }, { 0.04f, 1.0f, 0.75f }, house.doorColor);
}

}

Mesh houseWalls(const HouseDescription& house)
{
    Mesh mesh;

    float w = house.width;
    float d = house.depth;
    float h = house.height;
    float front = -d * 0.5f;

    mesh.addBox({ 0.0f, h * 0.5f, 0.0f }, { w, h, d }, house.wallColor);

    // Barrado (rodape) e cimalha
    mesh.addBox({ 0.0f, 0.25f, 0.0f }, { w + 0.06f, 0.5f, d + 0.06f }, darker(house.wallColor, 0.78f));
    mesh.addBox({ 0.0f, h - 0.06f, 0.0f }, { w + 0.16f, 0.14f, d + 0.16f }, house.trimColor);

    // Cunhais (pilastras nos cantos)
    for (int side = -1; side <= 1; side += 2) {
        mesh.addBox({ side * (w * 0.5f - 0.1f), h * 0.5f, front - 0.02f }, { 0.24f, h, 0.06f }, house.trimColor);
    }

    // Porta com moldura e degrau
    float doorX = (house.floors > 1) ? 0.0f : -w * 0.18f;
    mesh.addBox({ doorX, 1.15f, front - 0.025f }, { 1.3f, 2.35f, 0.05f }, house.trimColor);
    mesh.addBox({ doorX, 1.08f, front - 0.06f }, { 1.05f, 2.15f, 0.05f }, house.doorColor);
    mesh.addBox({ doorX + 0.32f, 1.05f, front - 0.095f }, { 0.06f, 0.06f, 0.04f }, { 0.85f, 0.75f, 0.35f });
    mesh.addBox({ doorX, 0.06f, front - 0.3f }, { 1.5f, 0.12f, 0.5f }, darker(house.wallColor, 0.7f));

    // Janelas do terreo
    if (house.floors > 1) {
        addFrontWindow(mesh, -w * 0.32f, 1.6f, front, house);
        addFrontWindow(mesh, w * 0.32f, 1.6f, front, house);
    }
    else {
        addFrontWindow(mesh, w * 0.22f, 1.6f, front, house);
        if (w > 6.5f) {
            addFrontWindow(mesh, w * 0.40f - 0.3f, 1.6f, front, house);
        }
    }

    // Andar de cima: janelas e sacadas
    if (house.floors > 1) {
        float y = h * 0.5f + 1.55f;
        for (int i = -1; i <= 1; ++i) {
            float x = i * w * 0.32f;
            addFrontWindow(mesh, x, y, front, house);
            mesh.addBox({ x, y - 0.8f, front - 0.3f }, { 1.3f, 0.08f, 0.6f }, house.trimColor);
            mesh.addBox({ x, y - 0.5f, front - 0.58f }, { 1.3f, 0.05f, 0.04f }, { 0.15f, 0.15f, 0.15f });
            for (int b = 0; b < 7; ++b) {
                mesh.addBox({ x - 0.6f + b * 0.2f, y - 0.62f, front - 0.58f }, { 0.025f, 0.28f, 0.025f }, { 0.15f, 0.15f, 0.15f });
            }
        }
        mesh.addBox({ 0.0f, h * 0.5f + 0.2f, front - 0.04f }, { w + 0.04f, 0.12f, 0.08f }, house.trimColor);
    }

    // Janelas laterais
    for (int side = -1; side <= 1; side += 2) {
        addSideWindow(mesh, side * w * 0.5f, 1.6f, 0.0f, static_cast<float>(side), house);
    }

    if (house.roof == RoofStyle::Gable) {
        // Oitoes (triangulos das paredes laterais)
        float ridge = h + 0.12f + d * 0.32f;
        Vec3 color = house.wallColor;
        mesh.addTriangle({ -w * 0.5f, h, -d * 0.5f }, { -w * 0.5f, h, d * 0.5f }, { -w * 0.5f, ridge, 0.0f }, color);
        mesh.addTriangle({ w * 0.5f, h, -d * 0.5f }, { w * 0.5f, ridge, 0.0f }, { w * 0.5f, h, d * 0.5f }, color);
    }
    else {
        // Platibanda: a fachada sobe e esconde o telhado
        mesh.addBox({ 0.0f, h + 0.45f, front + 0.1f }, { w + 0.04f, 0.9f, 0.2f }, house.wallColor);
        mesh.addBox({ 0.0f, h + 0.93f, front + 0.1f }, { w + 0.2f, 0.1f, 0.3f }, house.trimColor);
        mesh.addBox({ 0.0f, h + 1.15f, front + 0.1f }, { w * 0.32f, 0.35f, 0.2f }, house.wallColor);
        mesh.addBox({ 0.0f, h + 1.36f, front + 0.1f }, { w * 0.36f, 0.08f, 0.28f }, house.trimColor);

        // Paredes laterais acompanhando a queda do telhado
        float y1 = h + 0.65f;
        float zf = front + 0.2f;
        float zb = d * 0.5f;
        mesh.addTriangle({ -w * 0.5f, h, zf }, { -w * 0.5f, h, zb }, { -w * 0.5f, y1, zf }, house.wallColor);
        mesh.addTriangle({ w * 0.5f, h, zf }, { w * 0.5f, y1, zf }, { w * 0.5f, h, zb }, house.wallColor);
    }

    return mesh;
}

Mesh houseRoof(const HouseDescription& house)
{
    Mesh mesh;

    float w = house.width;
    float d = house.depth;
    float h = house.height;
    Vec3 color = house.roofColor;

    if (house.roof == RoofStyle::Gable) {
        float x = w * 0.5f + 0.3f;
        float z = d * 0.5f + 0.4f;
        float eave = h + 0.12f - 0.4f * 0.32f;
        float ridge = h + 0.12f + d * 0.32f;

        // Duas aguas, com fileiras de telhas (faixas de tons alternados)
        const int rows = 6;
        for (int i = 0; i < rows; ++i) {
            float t0 = static_cast<float>(i) / rows;
            float t1 = static_cast<float>(i + 1) / rows;
            Vec3 tone = darker(color, (i % 2 == 0) ? 1.0f : 0.9f);

            float y0 = lerp(eave, ridge, t0), y1 = lerp(eave, ridge, t1);
            float zf0 = lerp(-z, 0.0f, t0), zf1 = lerp(-z, 0.0f, t1);
            float zb0 = lerp(z, 0.0f, t0), zb1 = lerp(z, 0.0f, t1);

            mesh.addQuad({ -x, y0, zf0 }, { -x, y1, zf1 }, { x, y1, zf1 }, { x, y0, zf0 }, tone, 1.0f, 1.0f);
            mesh.addQuad({ -x, y0, zb0 }, { x, y0, zb0 }, { x, y1, zb1 }, { -x, y1, zb1 }, tone, 1.0f, 1.0f);
        }

        // Cumeeira
        mesh.addBox({ 0.0f, ridge + 0.04f, 0.0f }, { 2.0f * x + 0.05f, 0.12f, 0.22f }, darker(color, 0.8f));
    }
    else {
        float x = w * 0.5f + 0.05f;
        float y1 = h + 0.65f;
        float y0 = h + 0.1f;
        float zf = -d * 0.5f + 0.2f;
        float zb = d * 0.5f + 0.35f;

        mesh.addQuad({ -x, y1, zf }, { -x, y0, zb }, { x, y0, zb }, { x, y1, zf }, color, 1.0f, 1.0f);
    }

    return mesh;
}

Mesh pier(float width, float length, float deckHeight, float railGapStart, float railGapEnd)
{
    Mesh mesh;

    const Vec3 plank = { 0.66f, 0.50f, 0.36f };
    const Vec3 beam = { 0.48f, 0.36f, 0.26f };
    const Vec3 post = { 0.40f, 0.30f, 0.22f };

    // Tabuas do deck
    const float plankDepth = 0.3f;
    const float gap = 0.035f;
    int planks = static_cast<int>(length / (plankDepth + gap));

    for (int i = 0; i < planks; ++i) {
        float z = -(i + 0.5f) * (plankDepth + gap);
        float tone = 0.86f + 0.24f * random01(i, 7);
        float wobble = 0.08f * (random01(i, 9) - 0.5f);
        mesh.addBox({ wobble, deckHeight - 0.035f, z }, { width, 0.07f, plankDepth }, darker(plank, tone));
    }

    // Vigas longitudinais e travessas
    for (int side = -1; side <= 1; side += 2) {
        mesh.addBox({ side * width * 0.32f, deckHeight - 0.15f, -length * 0.5f }, { 0.16f, 0.16f, length }, beam);
    }

    const float spacing = 2.5f;
    int posts = static_cast<int>(length / spacing) + 1;

    for (int i = 0; i < posts; ++i) {
        float z = -i * spacing - 0.2f;
        mesh.addBox({ 0.0f, deckHeight - 0.28f, z }, { width + 0.2f, 0.12f, 0.16f }, beam);

        for (int side = -1; side <= 1; side += 2) {
            bool inGap = side > 0 && z < railGapStart && z > railGapEnd;
            float top = inGap ? deckHeight + 0.3f : deckHeight + 0.95f;
            float x = side * (width * 0.5f + 0.06f);
            mesh.addFrustum({ x, -2.4f, z }, { x, top, z }, 0.12f, 0.1f, 7, post);
        }
    }

    // Guarda-corpo (dois corrimaos), interrompido onde o barco atraca
    for (int i = 0; i + 1 < posts; ++i) {
        float z0 = -i * spacing - 0.2f;
        float z1 = -(i + 1) * spacing - 0.2f;

        for (int side = -1; side <= 1; side += 2) {
            bool inGap = side > 0 && z0 <= railGapStart + 0.01f && z1 >= railGapEnd - 0.01f;
            if (side > 0 && ((z0 < railGapStart && z0 > railGapEnd) || (z1 < railGapStart && z1 > railGapEnd))) {
                inGap = true;
            }
            if (inGap) continue;

            float x = side * (width * 0.5f + 0.06f);
            mesh.addBox({ x, deckHeight + 0.9f, (z0 + z1) * 0.5f }, { 0.09f, 0.08f, spacing }, plank);
            mesh.addBox({ x, deckHeight + 0.5f, (z0 + z1) * 0.5f }, { 0.06f, 0.06f, spacing }, beam);
        }
    }

    return mesh;
}

Mesh streetLamp()
{
    Mesh mesh;

    const Vec3 metal = { 0.16f, 0.30f, 0.26f };

    mesh.addFrustum({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.45f, 0.0f }, 0.16f, 0.12f, 8, metal);
    mesh.addFrustum({ 0.0f, 0.45f, 0.0f }, { 0.0f, 3.5f, 0.0f }, 0.07f, 0.05f, 8, metal);
    mesh.addFrustum({ 0.0f, 3.45f, 0.0f }, { 0.35f, 3.75f, 0.0f }, 0.04f, 0.035f, 6, metal);
    mesh.addFrustum({ 0.35f, 3.75f, 0.0f }, { 0.65f, 3.7f, 0.0f }, 0.035f, 0.035f, 6, metal);

    // Luminaria
    mesh.addFrustum({ 0.65f, 3.78f, 0.0f }, { 0.65f, 3.55f, 0.0f }, 0.05f, 0.24f, 8, metal);
    mesh.addSphere({ 0.65f, 3.52f, 0.0f }, { 0.13f, 0.08f, 0.13f }, 8, 4, { 1.0f, 0.95f, 0.75f });

    return mesh;
}

Mesh woodenSign()
{
    Mesh mesh;

    const Vec3 wood = { 0.62f, 0.46f, 0.32f };
    const Vec3 dark = { 0.40f, 0.28f, 0.19f };
    const Vec3 paint = { 0.20f, 0.36f, 0.55f };

    for (int side = -1; side <= 1; side += 2) {
        mesh.addFrustum({ side * 0.62f, 0.0f, 0.0f }, { side * 0.62f, 1.75f, 0.0f }, 0.06f, 0.05f, 6, dark);
    }

    mesh.addBox({ 0.0f, 1.3f, 0.0f }, { 1.5f, 0.75f, 0.08f }, wood);
    mesh.addBox({ 0.0f, 1.69f, 0.0f }, { 1.6f, 0.06f, 0.12f }, dark);
    mesh.addBox({ 0.0f, 0.91f, 0.0f }, { 1.6f, 0.06f, 0.12f }, dark);

    // "Letras" pintadas (faixas) e um sol estilizado
    const float widths[3] = { 1.1f, 0.85f, 0.95f };
    for (int i = 0; i < 3; ++i) {
        mesh.addBox({ -0.12f + (i == 1 ? -0.12f : 0.0f), 1.48f - i * 0.17f, 0.045f }, { widths[i], 0.07f, 0.01f }, paint);
    }
    mesh.addSphere({ 0.55f, 1.46f, 0.05f }, { 0.09f, 0.09f, 0.02f }, 8, 3, { 0.95f, 0.7f, 0.2f });

    return mesh;
}

}
