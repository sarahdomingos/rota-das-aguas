#ifndef NPC_H
#define NPC_H

// Personagem secundario (NPC). Usa o mesmo gerador de modelo da Lia, com a
// aparencia definida por um CharacterStyle (pele, cabelo, roupa, altura e
// acessorio). Fica parado "respirando", vira para a Lia quando ela chega
// perto e, ao interagir, abre o dialogo com o id dado (assets/dialogos).

#include "Entity.h"
#include "models/LiaModel.h"

#include <string>

class NPC : public Entity {
public:
    NPC(const std::string& name, const CharacterStyle& style, const std::string& dialogueId);

    void place(const Vec3& position, float yawDegrees);

    void update(float deltaTime, const Vec3& playerPosition);
    void draw(Renderer& renderer, float time) const override;

    Vec3 getPosition() const;
    float getRadius() const;
    const std::string& getName() const;
    const std::string& getDialogueId() const;

    void setHighlight(float value);

private:
    std::string m_name;
    std::string m_dialogueId;
    CharacterModel m_model;
    float m_height;
    float m_baseYaw;
    float m_idlePhase;
    float m_highlight;
};

#endif
