#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"

// Centraliza o desenho: ativa o shader, envia as matrizes de camera
// (view/projection) uma vez por quadro e a matriz de modelo por objeto.
class Renderer {
public:
    Renderer();

    bool init();

    void beginFrame(const Camera& camera);

    void draw(const Mesh& mesh, const float modelMatrix[16], const Vec3& color);
    void draw(const Model& model, const float modelMatrix[16], const Vec3& color);

private:
    Shader m_shader;

    void setObjectUniforms(const float modelMatrix[16], const Vec3& color);
};

#endif
