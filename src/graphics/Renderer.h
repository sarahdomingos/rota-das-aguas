#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"

// Como um objeto deve ser pintado (cor, textura e brilho do modelo de Phong).
struct Material {
    Vec3 color = { 1.0f, 1.0f, 1.0f };
    const Texture* texture = nullptr;
    float textureScale = 1.0f;
    bool worldUV = false;       // textura projetada de cima (terreno)
    float specular = 0.06f;     // intensidade do brilho especular
    float shininess = 12.0f;    // "dureza" do brilho
    float highlight = 0.0f;     // destaque de objeto interativo (0 a 1)
    float alpha = 1.0f;
    bool unlit = false;         // ignora a luz (ceu, sombras)
    bool twoSided = false;      // ilumina os dois lados (folhas)
    bool water = false;         // ondas + transparencia + reflexo
    bool fog = true;
};

// Centraliza o desenho: ativa o shader, envia camera, luz e tempo uma vez
// por quadro, e a matriz de modelo + material para cada objeto.
class Renderer {
public:
    Renderer();

    bool init();

    void beginFrame(const Camera& camera, const Light& light, float time);

    void drawSky(const Mesh& dome, const Camera& camera);

    void draw(const Mesh& mesh, const float modelMatrix[16], const Material& material);
    void draw(const Model& model, const float modelMatrix[16], const Material& material);

    // Objetos transparentes (agua, sombras) sao desenhados por ultimo.
    void beginTransparent();
    void endTransparent();

    // Desenho 2D por cima da cena (interface): coordenadas em pixels, origem no
    // canto superior esquerdo, sem teste de profundidade e com transparencia.
    void begin2D(int width, int height);
    void setMaterial2D(const Texture* texture);
    void end2D();

    // A agua nao e desenhada dentro do casco do barco (veja basic.frag).
    void setBoatMask(const float boatMatrix[16]);
    void clearBoatMask();

private:
    Shader m_shader;

    void setObjectUniforms(const float modelMatrix[16], const Material& material);
};

#endif
