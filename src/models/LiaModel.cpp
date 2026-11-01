#include "LiaModel.h"

#include "graphics/ProceduralTextures.h"
#include "graphics/Renderer.h"

#include <cmath>

using namespace MathUtils;

namespace {

// Paleta da Lia
const Vec3 BAG = { 0.80f, 0.30f, 0.22f };
const Vec3 BAG_DARK = { 0.58f, 0.20f, 0.15f };
const Vec3 STRAP = { 0.45f, 0.27f, 0.16f };
const Vec3 EYES = { 0.07f, 0.045f, 0.035f };
const Vec3 MOUTH = { 0.45f, 0.20f, 0.17f };
const Vec3 BLUSH = { 0.72f, 0.40f, 0.36f };

Vec3 darker(const Vec3& c)
{
    return { c.x * 0.85f, c.y * 0.82f, c.z * 0.8f };
}

// Texturas dos personagens, criadas uma vez (na primeira vez que alguem e
// desenhado, quando o OpenGL ja existe) e compartilhadas por todos.
const Texture* fabricTexture()
{
    static Texture* texture = nullptr;
    if (texture == nullptr) {
        texture = new Texture();
        texture->create(ProceduralTextures::SIZE, ProceduralTextures::SIZE, ProceduralTextures::fabric());
    }
    return texture;
}

const Texture* hairTexture()
{
    static Texture* texture = nullptr;
    if (texture == nullptr) {
        texture = new Texture();
        texture->create(ProceduralTextures::SIZE, ProceduralTextures::SIZE, ProceduralTextures::hair());
    }
    return texture;
}

void appendScaled(Mesh& target, const Mesh& source, const Vec3& scaleBy)
{
    float matrix[16];
    scaling(scaleBy.x, scaleBy.y, scaleBy.z, matrix);
    target.append(source, matrix);
}

void buildHead(CharacterPart& body, const CharacterStyle& s)
{
    const Vec3 headCenter = { 0.0f, 1.22f, 0.0f };
    Mesh& skin = body.skin;

    skin.addFrustum({ 0.0f, 0.95f, 0.0f }, { 0.0f, 1.10f, 0.0f }, 0.058f, 0.05f, 12, s.skin);
    skin.addSphere(headCenter, { 0.165f, 0.18f, 0.16f }, 20, 14, s.skin);

    // Orelhas
    skin.addSphere({ -0.163f, 1.21f, 0.0f }, { 0.025f, 0.04f, 0.03f }, 8, 6, darker(s.skin));
    skin.addSphere({ 0.163f, 1.21f, 0.0f }, { 0.025f, 0.04f, 0.03f }, 8, 6, darker(s.skin));

    // Rosto: olhos, sobrancelhas, nariz, boca e bochechas
    skin.addSphere({ -0.058f, 1.235f, -0.147f }, { 0.022f, 0.03f, 0.014f }, 8, 6, EYES);
    skin.addSphere({ 0.058f, 1.235f, -0.147f }, { 0.022f, 0.03f, 0.014f }, 8, 6, EYES);
    skin.addSphere({ -0.052f, 1.243f, -0.157f }, { 0.007f, 0.007f, 0.004f }, 6, 4, { 1.0f, 1.0f, 1.0f });
    skin.addSphere({ 0.064f, 1.243f, -0.157f }, { 0.007f, 0.007f, 0.004f }, 6, 4, { 1.0f, 1.0f, 1.0f });

    skin.addRoundedBox({ -0.06f, 1.285f, -0.138f }, { 0.055f, 0.012f, 0.012f }, 0.005f, s.hairColor, 2);
    skin.addRoundedBox({ 0.06f, 1.285f, -0.138f }, { 0.055f, 0.012f, 0.012f }, 0.005f, s.hairColor, 2);

    skin.addSphere({ 0.0f, 1.195f, -0.16f }, { 0.022f, 0.025f, 0.02f }, 8, 6, darker(s.skin));
    skin.addRoundedBox({ 0.0f, 1.145f, -0.142f }, { 0.055f, 0.012f, 0.012f }, 0.005f, MOUTH, 2);

    skin.addSphere({ -0.095f, 1.17f, -0.124f }, { 0.03f, 0.022f, 0.012f }, 8, 6, BLUSH);
    skin.addSphere({ 0.095f, 1.17f, -0.124f }, { 0.03f, 0.022f, 0.012f }, 8, 6, BLUSH);
}

void buildHair(CharacterPart& body, const CharacterStyle& s)
{
    Mesh& hair = body.hair;

    switch (s.hair) {
    case HairStyle::Long:
        // Cabelo liso: calota sobre a cabeca + bloco caindo atras ate os ombros
        hair.addSphere({ 0.0f, 1.27f, 0.045f }, { 0.182f, 0.185f, 0.172f }, 20, 12, s.hairColor);
        hair.addRoundedBox({ 0.0f, 1.12f, 0.1f }, { 0.34f, 0.3f, 0.13f }, 0.05f, s.hairColor);
        hair.addRoundedBox({ -0.16f, 1.16f, -0.02f }, { 0.05f, 0.22f, 0.2f }, 0.022f, s.hairColor);
        hair.addRoundedBox({ 0.16f, 1.16f, -0.02f }, { 0.05f, 0.22f, 0.2f }, 0.022f, s.hairColor);
        break;
    case HairStyle::Short:
        hair.addSphere({ 0.0f, 1.27f, 0.04f }, { 0.176f, 0.17f, 0.168f }, 20, 12, s.hairColor);
        break;
    case HairStyle::Bald:
        break;
    }
}

void buildTorso(CharacterPart& body, const CharacterStyle& s)
{
    // Short e camiseta, achatados na profundidade. A gola fecha arredondada.
    Mesh clothes;
    clothes.addFrustum({ 0.0f, 0.50f, 0.0f }, { 0.0f, 0.68f, 0.0f }, 0.165f, 0.152f, 18, s.shorts);
    const std::vector<Mesh::ProfilePoint> shirt = {
        { 0.00f, 0.150f }, { 0.70f, 0.164f }, { 0.86f, 0.168f }, { 0.94f, 0.150f }, { 1.00f, 0.090f }, { 1.00f, 0.000f }
    };
    clothes.addLathe({ 0.0f, 0.64f, 0.0f }, { 0.0f, 1.0f, 0.0f }, shirt, 18, s.shirt);
    appendScaled(body.cloth, clothes, { 1.0f, 1.0f, 0.78f });
}

void buildBackpack(CharacterPart& body)
{
    Mesh& cloth = body.cloth;
    cloth.addRoundedBox({ 0.0f, 0.80f, 0.185f }, { 0.25f, 0.30f, 0.12f }, 0.04f, BAG);
    cloth.addRoundedBox({ 0.0f, 0.925f, 0.18f }, { 0.262f, 0.075f, 0.135f }, 0.03f, BAG_DARK);

    cloth.addRoundedBox({ 0.0f, 0.74f, 0.252f }, { 0.17f, 0.11f, 0.025f }, 0.012f, BAG_DARK);

    // Alcas sobre os ombros e na frente do tronco
    for (int side = -1; side <= 1; side += 2) {
        cloth.addRoundedBox({ side * 0.085f, 0.995f, 0.04f }, { 0.04f, 0.025f, 0.25f }, 0.011f, STRAP, 2);
        cloth.addRoundedBox({ side * 0.085f, 0.86f, -0.123f }, { 0.04f, 0.26f, 0.02f }, 0.009f, STRAP, 2);
    }
}

void buildStrawHat(CharacterPart& body, const Vec3& color)
{
    // Chapeu de palha por revolucao: aba, copa e fita numa peca so
    const std::vector<Mesh::ProfilePoint> hat = {
        { 0.00f, 0.000f }, { 0.01f, 0.300f }, { 0.04f, 0.340f }, { 0.12f, 0.335f }, { 0.16f, 0.200f },
        { 0.25f, 0.192f }, { 0.80f, 0.165f }, { 0.94f, 0.140f }, { 1.00f, 0.000f }
    };
    body.cloth.addLathe({ 0.0f, 1.33f, 0.02f }, { 0.0f, 1.47f, 0.02f }, hat, 20, color);
    body.cloth.addFrustum({ 0.0f, 1.36f, 0.02f }, { 0.0f, 1.39f, 0.02f }, 0.196f, 0.192f, 20, darker(color), false);
}

void buildApron(CharacterPart& body, const Vec3& color)
{
    Mesh& cloth = body.cloth;
    cloth.addRoundedBox({ 0.0f, 0.70f, -0.135f }, { 0.26f, 0.36f, 0.02f }, 0.009f, color, 2);
    cloth.addRoundedBox({ 0.0f, 0.90f, -0.128f }, { 0.16f, 0.08f, 0.02f }, 0.009f, color, 2);
    cloth.addRoundedBox({ 0.0f, 0.79f, -0.115f }, { 0.31f, 0.03f, 0.24f }, 0.012f, darker(color), 2);
}

CharacterPart buildArm(const CharacterStyle& s)
{
    CharacterPart arm;
    // Manga com a ponta do ombro arredondada
    arm.cloth.addSphere({ 0.0f, 0.02f, 0.0f }, { 0.066f, 0.05f, 0.066f }, 12, 8, s.shirt);
    arm.cloth.addFrustum({ 0.0f, 0.03f, 0.0f }, { 0.0f, -0.14f, 0.0f }, 0.066f, 0.06f, 14, s.shirt);
    arm.skin.addFrustum({ 0.0f, -0.10f, 0.0f }, { 0.0f, -0.42f, 0.0f }, 0.046f, 0.039f, 12, s.skin);
    arm.skin.addSphere({ 0.0f, -0.455f, -0.005f }, { 0.043f, 0.05f, 0.04f }, 10, 8, s.skin);

    return arm;
}

CharacterPart buildLeg(const CharacterStyle& s)
{
    CharacterPart leg;
    leg.cloth.addFrustum({ 0.0f, 0.02f, 0.0f }, { 0.0f, -0.17f, 0.0f }, 0.085f, 0.078f, 14, s.shorts);
    leg.skin.addFrustum({ 0.0f, -0.15f, 0.0f }, { 0.0f, -0.53f, 0.0f }, 0.058f, 0.044f, 12, s.skin);
    leg.skin.addSphere({ 0.0f, -0.53f, 0.0f }, { 0.045f, 0.03f, 0.045f }, 10, 6, s.skin);

    // Pe com sandalia simples (sola arredondada)
    leg.cloth.addRoundedBox({ 0.0f, -0.55f, -0.04f }, { 0.09f, 0.05f, 0.2f }, 0.022f, s.shoes);

    return leg;
}

// Materiais de um personagem: pele lisa, roupa com tecido e cabelo brilhante
struct CharacterMaterials {
    Material skin;
    Material cloth;
    Material hair;
};

void drawMeshes(const CharacterPart& part, Renderer& renderer, const float matrix[16], const CharacterMaterials& materials)
{
    if (!part.skin.empty()) renderer.draw(part.skin, matrix, materials.skin);
    if (!part.cloth.empty()) renderer.draw(part.cloth, matrix, materials.cloth);
    if (!part.hair.empty()) renderer.draw(part.hair, matrix, materials.hair);
}

void drawPart(const CharacterPart& part, Renderer& renderer, const float parent[16], const Vec3& pivot,
              float rotationXDeg, float rotationZDeg, const CharacterMaterials& materials)
{
    float matrix[16];
    float local[16];

    for (int i = 0; i < 16; ++i) matrix[i] = parent[i];

    translation(pivot.x, pivot.y, pivot.z, local);
    apply(matrix, local);
    rotationZ(rotationZDeg, local);
    apply(matrix, local);
    rotationX(rotationXDeg, local);
    apply(matrix, local);

    drawMeshes(part, renderer, matrix, materials);
}

}

namespace LiaModel {

CharacterStyle liaStyle()
{
    CharacterStyle style;
    style.skin = { 0.56f, 0.37f, 0.26f };
    style.hairColor = { 0.12f, 0.075f, 0.055f };
    style.shirt = { 0.98f, 0.76f, 0.24f };
    style.shorts = { 0.23f, 0.40f, 0.62f };
    style.shoes = { 0.50f, 0.30f, 0.17f };
    style.hair = HairStyle::Long;
    style.accessory = Accessory::Backpack;
    style.accessoryColor = { 0.80f, 0.30f, 0.22f };
    style.height = 1.0f;
    return style;
}

CharacterModel build()
{
    return build(liaStyle());
}

CharacterModel build(const CharacterStyle& s)
{
    CharacterModel model;

    buildTorso(model.body, s);
    buildHead(model.body, s);
    buildHair(model.body, s);

    switch (s.accessory) {
    case Accessory::Backpack: buildBackpack(model.body); break;
    case Accessory::StrawHat: buildStrawHat(model.body, s.accessoryColor); break;
    case Accessory::Apron:    buildApron(model.body, s.accessoryColor); break;
    case Accessory::None:     break;
    }

    model.leftArm = buildArm(s);
    model.rightArm = buildArm(s);
    model.leftLeg = buildLeg(s);
    model.rightLeg = buildLeg(s);

    // Os membros sao desenhados relativos a estes pivos (ombros e quadril)
    model.leftShoulder = { -0.205f, 0.965f, 0.0f };
    model.rightShoulder = { 0.205f, 0.965f, 0.0f };
    model.leftHip = { -0.085f, 0.58f, 0.0f };
    model.rightHip = { 0.085f, 0.58f, 0.0f };

    return model;
}

void draw(const CharacterModel& model, Renderer& renderer, const float root[16],
          float walkPhase, float walkAmount, float time, float highlight)
{
    CharacterMaterials materials;
    materials.skin.highlight = highlight;
    materials.skin.specular = 0.12f;
    materials.skin.shininess = 20.0f;

    materials.cloth.highlight = highlight;
    materials.cloth.texture = fabricTexture();
    materials.cloth.textureScale = 2.5f;
    materials.cloth.specular = 0.04f;
    materials.cloth.shininess = 8.0f;

    materials.hair.highlight = highlight;
    materials.hair.texture = hairTexture();
    materials.hair.textureScale = 1.0f;
    materials.hair.specular = 0.3f;
    materials.hair.shininess = 28.0f;

    float swing = std::sin(walkPhase);
    float legAngle = 32.0f * swing * std::fmin(walkAmount, 1.4f);
    float armAngle = 26.0f * swing * std::fmin(walkAmount, 1.4f);

    // Respiracao parada e "quique" do corpo ao andar
    float breathe = 0.006f * std::sin(time * 2.2f) * (1.0f - std::fmin(walkAmount, 1.0f));
    float bounce = 0.035f * std::fabs(std::cos(walkPhase)) * std::fmin(walkAmount, 1.4f);
    float lean = 4.0f * std::fmin(walkAmount, 1.5f);

    float hips[16];
    float local[16];
    for (int i = 0; i < 16; ++i) hips[i] = root[i];

    translation(0.0f, bounce + breathe, 0.0f, local);
    apply(hips, local);

    // Pernas presas ao quadril (sem inclinacao do tronco)
    drawPart(model.leftLeg, renderer, hips, model.leftHip, legAngle, 0.0f, materials);
    drawPart(model.rightLeg, renderer, hips, model.rightHip, -legAngle, 0.0f, materials);

    // Tronco levemente inclinado para frente ao andar (gira em torno do quadril)
    float torso[16];
    for (int i = 0; i < 16; ++i) torso[i] = hips[i];
    translation(0.0f, 0.58f, 0.0f, local);
    apply(torso, local);
    rotationX(-lean, local);
    apply(torso, local);
    translation(0.0f, -0.58f, 0.0f, local);
    apply(torso, local);

    drawMeshes(model.body, renderer, torso, materials);

    float idleArm = 2.5f * std::sin(time * 1.6f);
    drawPart(model.leftArm, renderer, torso, model.leftShoulder, -armAngle + idleArm, -6.0f, materials);
    drawPart(model.rightArm, renderer, torso, model.rightShoulder, armAngle - idleArm, 6.0f, materials);
}

}
