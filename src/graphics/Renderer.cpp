#include "Renderer.h"

#include "core/AssetPath.h"

#include <GLFW/glfw3.h>

Renderer::Renderer()
{
}

bool Renderer::init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return m_shader.loadFromFiles(
        AssetPath::resolve("assets/shaders/basic.vert"),
        AssetPath::resolve("assets/shaders/basic.frag")
    );
}

void Renderer::beginFrame(const Camera& camera)
{
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Ceu
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float view[16];
    float projection[16];

    camera.getViewMatrix(view);
    camera.getProjectionMatrix(projection);

    m_shader.use();
    m_shader.setMat4("u_view", view);
    m_shader.setMat4("u_projection", projection);
    m_shader.setVec3("u_shadeDirection", { 0.4f, 1.0f, 0.3f });
}

void Renderer::setObjectUniforms(const float modelMatrix[16], const Vec3& color)
{
    m_shader.setMat4("u_model", modelMatrix);
    m_shader.setVec4("u_tint", color.x, color.y, color.z, 1.0f);
}

void Renderer::draw(const Mesh& mesh, const float modelMatrix[16], const Vec3& color)
{
    setObjectUniforms(modelMatrix, color);
    mesh.draw();
}

void Renderer::draw(const Model& model, const float modelMatrix[16], const Vec3& color)
{
    setObjectUniforms(modelMatrix, color);
    model.draw();
}
