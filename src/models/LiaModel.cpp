#include "LiaModel.h"

#include "graphics/Renderer.h"

#include <cmath>

using namespace MathUtils;

namespace {

// Paleta da Lia
const Vec3 SKIN = { 0.56f, 0.37f, 0.26f };
const Vec3 SKIN_SHADE = { 0.48f, 0.30f, 0.21f };
const Vec3 HAIR = { 0.12f, 0.075f, 0.055f };
const Vec3 SHIRT = { 0.98f, 0.76f, 0.24f };
const Vec3 SHORTS = { 0.23f, 0.40f, 0.62f };
const Vec3 BAG = { 0.80f, 0.30f, 0.22f };
const Vec3 BAG_DARK = { 0.58f, 0.20f, 0.15f };
const Vec3 STRAP = { 0.45f, 0.27f, 0.16f };
const Vec3 SANDAL = { 0.50f, 0.30f, 0.17f };
const Vec3 EYES = { 0.07f, 0.045f, 0.035f };
const Vec3 MOUTH = { 0.45f, 0.20f, 0.17f };
const Vec3 BLUSH = { 0.72f, 0.40f, 0.36f };

void appendScaled(Mesh& target, const Mesh& source, const Vec3& scaleBy)
{
    float matrix[16];
    scaling(scaleBy.x, scaleBy.y, scaleBy.z, matrix);
    target.append(source, matrix);
}

void buildHead(Mesh& body)
{
    const Vec3 headCenter = { 0.0f, 1.22f, 0.0f };

    body.addFrustum({ 0.0f, 0.97f, 0.0f }, { 0.0f, 1.08f, 0.0f }, 0.055f, 0.05f, 6, SKIN);
    body.addSphere(headCenter, { 0.165f, 0.18f, 0.16f }, 12, 9, SKIN);

    // Orelhas
    body.addSphere({ -0.163f, 1.21f, 0.0f }, { 0.025f, 0.04f, 0.03f }, 6, 4, SKIN_SHADE);
    body.addSphere({ 0.163f, 1.21f, 0.0f }, { 0.025f, 0.04f, 0.03f }, 6, 4, SKIN_SHADE);

    // Rosto: olhos, sobrancelhas, nariz, boca e bochechas
    body.addSphere({ -0.058f, 1.235f, -0.147f }, { 0.022f, 0.03f, 0.014f }, 6, 4, EYES);
    body.addSphere({ 0.058f, 1.235f, -0.147f }, { 0.022f, 0.03f, 0.014f }, 6, 4, EYES);
    body.addSphere({ -0.052f, 1.243f, -0.157f }, { 0.007f, 0.007f, 0.004f }, 4, 3, { 1.0f, 1.0f, 1.0f });
    body.addSphere({ 0.064f, 1.243f, -0.157f }, { 0.007f, 0.007f, 0.004f }, 4, 3, { 1.0f, 1.0f, 1.0f });

    body.addBox({ -0.06f, 1.285f, -0.138f }, { 0.055f, 0.012f, 0.012f }, HAIR);
    body.addBox({ 0.06f, 1.285f, -0.138f }, { 0.055f, 0.012f, 0.012f }, HAIR);

    body.addSphere({ 0.0f, 1.195f, -0.16f }, { 0.022f, 0.025f, 0.02f }, 6, 4, SKIN_SHADE);
    body.addBox({ 0.0f, 1.145f, -0.142f }, { 0.055f, 0.012f, 0.012f }, MOUTH);

    body.addSphere({ -0.095f, 1.17f, -0.124f }, { 0.03f, 0.022f, 0.012f }, 6, 4, BLUSH);
    body.addSphere({ 0.095f, 1.17f, -0.124f }, { 0.03f, 0.022f, 0.012f }, 6, 4, BLUSH);
}

void buildHair(Mesh& body)
{
    // Cabelo liso: calota sobre a cabeca + bloco caindo atras ate os ombros
    body.addSphere({ 0.0f, 1.27f, 0.045f }, { 0.182f, 0.185f, 0.172f }, 12, 8, HAIR);
    body.addBox({ 0.0f, 1.12f, 0.1f }, { 0.34f, 0.3f, 0.13f }, HAIR);
    body.addBox({ -0.16f, 1.16f, -0.02f }, { 0.05f, 0.22f, 0.2f }, HAIR);
    body.addBox({ 0.16f, 1.16f, -0.02f }, { 0.05f, 0.22f, 0.2f }, HAIR);
}

void buildTorso(Mesh& body)
{
    // Short e camiseta, achatados na profundidade
    Mesh clothes;
    clothes.addFrustum({ 0.0f, 0.50f, 0.0f }, { 0.0f, 0.68f, 0.0f }, 0.165f, 0.152f, 10, SHORTS);
    clothes.addFrustum({ 0.0f, 0.64f, 0.0f }, { 0.0f, 0.98f, 0.0f }, 0.15f, 0.168f, 10, SHIRT);
    clothes.addFrustum({ 0.0f, 0.98f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.168f, 0.09f, 10, SHIRT);
    appendScaled(body, clothes, { 1.0f, 1.0f, 0.78f });
}

void buildBackpack(Mesh& body)
{
    body.addBox({ 0.0f, 0.80f, 0.185f }, { 0.25f, 0.30f, 0.12f }, BAG);
    body.addBox({ 0.0f, 0.925f, 0.18f }, { 0.262f, 0.075f, 0.135f }, BAG_DARK);

    body.addBox({ 0.0f, 0.74f, 0.252f }, { 0.17f, 0.11f, 0.025f }, BAG_DARK);

    // Alcas sobre os ombros e na frente do tronco
    for (int side = -1; side <= 1; side += 2) {
        body.addBox({ side * 0.085f, 0.995f, 0.04f }, { 0.04f, 0.025f, 0.25f }, STRAP);
        body.addBox({ side * 0.085f, 0.86f, -0.123f }, { 0.04f, 0.26f, 0.02f }, STRAP);
    }
}

Mesh buildArm()
{
    Mesh arm;
    arm.addFrustum({ 0.0f, 0.03f, 0.0f }, { 0.0f, -0.14f, 0.0f }, 0.066f, 0.06f, 8, SHIRT);
    arm.addFrustum({ 0.0f, -0.10f, 0.0f }, { 0.0f, -0.42f, 0.0f }, 0.046f, 0.039f, 7, SKIN);
    arm.addSphere({ 0.0f, -0.455f, -0.005f }, { 0.043f, 0.05f, 0.04f }, 6, 4, SKIN);

    return arm;
}

Mesh buildLeg()
{
    Mesh leg;
    leg.addFrustum({ 0.0f, 0.02f, 0.0f }, { 0.0f, -0.17f, 0.0f }, 0.085f, 0.078f, 8, SHORTS);
    leg.addFrustum({ 0.0f, -0.15f, 0.0f }, { 0.0f, -0.53f, 0.0f }, 0.058f, 0.044f, 7, SKIN);

    // Pe com sandalia simples
    leg.addBox({ 0.0f, -0.55f, -0.04f }, { 0.09f, 0.05f, 0.2f }, SANDAL);

    return leg;
}

void drawPart(const Mesh& mesh, Renderer& renderer, const float parent[16], const Vec3& pivot,
              float rotationXDeg, float rotationZDeg, const Material& material)
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

    renderer.draw(mesh, matrix, material);
}

}

namespace LiaModel {

CharacterModel build()
{
    CharacterModel model;

    buildTorso(model.body);
    buildHead(model.body);
    buildHair(model.body);
    buildBackpack(model.body);

    model.leftArm = buildArm();
    model.rightArm = buildArm();
    model.leftLeg = buildLeg();
    model.rightLeg = buildLeg();

    // Os membros sao desenhados relativos a estes pivos (ombros e quadril)
    model.leftShoulder = { -0.205f, 0.965f, 0.0f };
    model.rightShoulder = { 0.205f, 0.965f, 0.0f };
    model.leftHip = { -0.085f, 0.58f, 0.0f };
    model.rightHip = { 0.085f, 0.58f, 0.0f };

    return model;
}

void draw(const CharacterModel& model, Renderer& renderer, const float root[16],
          float walkPhase, float walkAmount, float time)
{
    Material skin;
    skin.specular = 0.12f;
    skin.shininess = 20.0f;

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
    drawPart(model.leftLeg, renderer, hips, model.leftHip, legAngle, 0.0f, skin);
    drawPart(model.rightLeg, renderer, hips, model.rightHip, -legAngle, 0.0f, skin);

    // Tronco levemente inclinado para frente ao andar (gira em torno do quadril)
    float torso[16];
    for (int i = 0; i < 16; ++i) torso[i] = hips[i];
    translation(0.0f, 0.58f, 0.0f, local);
    apply(torso, local);
    rotationX(-lean, local);
    apply(torso, local);
    translation(0.0f, -0.58f, 0.0f, local);
    apply(torso, local);

    renderer.draw(model.body, torso, skin);

    float idleArm = 2.5f * std::sin(time * 1.6f);
    drawPart(model.leftArm, renderer, torso, model.leftShoulder, -armAngle + idleArm, -6.0f, skin);
    drawPart(model.rightArm, renderer, torso, model.rightShoulder, armAngle - idleArm, 6.0f, skin);
}

}
