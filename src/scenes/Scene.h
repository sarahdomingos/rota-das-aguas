#ifndef SCENE_H
#define SCENE_H

// Interface de uma cena (cidade) do jogo. Cada cidade (Maceio, Coqueiro Seco,
// Marechal Deodoro, Penedo...) implementa o seu terreno, objetos, colisoes,
// luz e interacoes; o Game so conversa com esta interface.

#include "graphics/Light.h"
#include "gameplay/Items.h"
#include "graphics/MathUtils.h"

#include <string>

class Renderer;
class Camera;

enum class Surface {
    Sand,
    Wood,
    Stone
};

enum class InteractionSound {
    Chime,
    Leaves,
    Pickup
};

struct Interaction {
    std::string dialogueId;     // bloco do arquivo de dialogos da cena (ou vazio)
    InteractionSound sound;
    std::string itemId;         // item pego (ou vazio)
    int itemQuantity;
};

class Scene {
public:
    virtual ~Scene() {}

    virtual bool init() = 0;
    virtual void update(float deltaTime, float time, const Vec3& playerPosition) = 0;

    virtual void drawSky(Renderer& renderer, const Camera& camera, float time) = 0;
    virtual void drawOpaque(Renderer& renderer, float time) = 0;
    virtual void drawShadows(Renderer& renderer, const Vec3& playerPosition) = 0;
    virtual void drawWater(Renderer& renderer) = 0;

    virtual const Light& getLight() const = 0;
    virtual Vec3 getPlayerStart() const = 0;

    // Mundo fisico: altura do chao, tipo de piso e colisao
    virtual float groundHeight(float x, float z) const = 0;
    virtual Surface surfaceAt(float x, float z) const = 0;
    virtual void resolveMovement(Vec3& position, const Vec3& previous, float radius) const = 0;

    // Distancia ate a beira-mar (para o volume das ondas)
    virtual float distanceToSea(const Vec3& position) const = 0;

    // Interacao: escolhe o objeto em foco (perto da Lia ou clicado) e interage
    virtual int findInteractable(const Vec3& playerPosition, const Vec3* rayOrigin, const Vec3* rayDirection) const = 0;
    virtual void setFocus(int id) = 0;
    virtual Interaction interact(int id) = 0;

    // Catalogo dos itens da cena (nomes, categorias e cores)
    virtual const ItemCatalog& getItemCatalog() const = 0;
};

#endif
