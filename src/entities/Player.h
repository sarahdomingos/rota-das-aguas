#ifndef PLAYER_H
#define PLAYER_H

// Lia, a personagem principal: anda na direcao pedida (relativa a camera),
// vira suavemente para onde anda, respeita o chao e as colisoes da cena e
// anima bracos e pernas.

#include "Entity.h"
#include "models/LiaModel.h"

class Scene;

class Player : public Entity {
public:
    Player();

    void build();
    void placeAt(const Vec3& position, float yawDegrees);

    // moveDirection: direcao no plano XZ (tamanho 0 = parada).
    void update(float deltaTime, const Vec3& moveDirection, bool running, const Scene& scene);
    void draw(Renderer& renderer, float time) const override;

    Vec3 getPosition() const;
    float getYaw() const;
    float getRadius() const;

    // Verdadeiro no quadro em que um pe toca o chao (som de passo).
    bool tookStep() const;

private:
    CharacterModel m_model;

    float m_walkPhase;
    float m_walkAmount;
    bool m_stepped;
};

#endif
