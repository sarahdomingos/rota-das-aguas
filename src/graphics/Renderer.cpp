#include "Renderer.h"

#include "core/AssetPath.h"

#include <GLFW/glfw3.h>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

Renderer::Renderer()
{
}

bool Renderer::init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!m_shader.loadFromFiles(AssetPath::resolve("assets/shaders/basic.vert"),
                                AssetPath::resolve("assets/shaders/basic.frag"))) {
        return false;
    }

    m_shader.use();
    m_shader.setInt("u_texture", 0);
    m_shader.setFloat("u_boatMask", 0.0f);

    return true;
}

void Renderer::beginFrame(const Camera& camera, const Light& light, float time)
{
    glClearColor(light.fogColor.x, light.fogColor.y, light.fogColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float view[16];
    float projection[16];

    camera.getViewMatrix(view);
    camera.getProjectionMatrix(projection);

    m_shader.use();
    m_shader.setMat4("u_view", view);
    m_shader.setMat4("u_projection", projection);
    m_shader.setVec3("u_cameraPos", camera.getPosition());
    m_shader.setFloat("u_time", time);

    m_shader.setVec3("u_sunDir", MathUtils::normalize(light.sunDirection));
    m_shader.setVec3("u_sunColor", light.sunColor);
    m_shader.setVec3("u_skyAmbient", light.skyAmbient);
    m_shader.setVec3("u_groundAmbient", light.groundAmbient);
    m_shader.setVec3("u_fogColor", light.fogColor);
    m_shader.setFloat("u_fogDensity", light.fogDensity);
}

void Renderer::drawSky(const Mesh& dome, const Camera& camera)
{
    // A cupula acompanha a camera e nao grava profundidade: fica sempre "atras" de tudo.
    Vec3 eye = camera.getPosition();

    float matrix[16];
    MathUtils::translation(eye.x, eye.y - 5.0f, eye.z, matrix);

    float scaleMatrix[16];
    MathUtils::scaling(150.0f, 150.0f, 150.0f, scaleMatrix);
    MathUtils::apply(matrix, scaleMatrix);

    Material sky;
    sky.unlit = true;
    sky.fog = false;

    glDepthMask(GL_FALSE);
    draw(dome, matrix, sky);
    glDepthMask(GL_TRUE);
}

void Renderer::setObjectUniforms(const float modelMatrix[16], const Material& material)
{
    float normals[16];
    MathUtils::normalMatrix(modelMatrix, normals);

    m_shader.setMat4("u_model", modelMatrix);
    m_shader.setMat4("u_normalMatrix", normals);
    m_shader.setVec4("u_color", material.color.x, material.color.y, material.color.z, material.alpha);

    bool textured = material.texture != nullptr && material.texture->isValid();
    if (textured) {
        material.texture->bind();
    }

    m_shader.setFloat("u_useTexture", textured ? 1.0f : 0.0f);
    m_shader.setFloat("u_textureScale", material.textureScale);
    m_shader.setFloat("u_worldUV", material.worldUV ? 1.0f : 0.0f);
    m_shader.setFloat("u_specular", material.specular);
    m_shader.setFloat("u_shininess", material.shininess);
    m_shader.setFloat("u_highlight", material.highlight);
    m_shader.setFloat("u_unlit", material.unlit ? 1.0f : 0.0f);
    m_shader.setFloat("u_twoSided", material.twoSided ? 1.0f : 0.0f);
    m_shader.setFloat("u_water", material.water ? 1.0f : 0.0f);
    m_shader.setFloat("u_useFog", material.fog ? 1.0f : 0.0f);
}

void Renderer::draw(const Mesh& mesh, const float modelMatrix[16], const Material& material)
{
    setObjectUniforms(modelMatrix, material);
    mesh.draw();
}

void Renderer::draw(const Model& model, const float modelMatrix[16], const Material& material)
{
    setObjectUniforms(modelMatrix, material);
    model.draw();
}

void Renderer::beginTransparent()
{
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
}

void Renderer::endTransparent()
{
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer::begin2D(int width, int height)
{
    float projection[16];
    float view[16];
    MathUtils::orthographic(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f, projection);
    MathUtils::identity(view);

    m_shader.setMat4("u_projection", projection);
    m_shader.setMat4("u_view", view);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}

void Renderer::setMaterial2D(const Texture* texture)
{
    float model[16];
    MathUtils::identity(model);

    Material material;
    material.unlit = true;
    material.fog = false;
    material.texture = texture;
    setObjectUniforms(model, material);
}

void Renderer::end2D()
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::setBoatMask(const float boatMatrix[16])
{
    float inverse[16];
    MathUtils::inverseAffine(boatMatrix, inverse);

    m_shader.setMat4("u_boatInverse", inverse);
    m_shader.setFloat("u_boatMask", 1.0f);
}

void Renderer::clearBoatMask()
{
    m_shader.setFloat("u_boatMask", 0.0f);
}
