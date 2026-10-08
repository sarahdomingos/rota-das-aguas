#ifndef ITEM_H
#define ITEM_H

// Item no mundo: gira e flutua levemente, brilha em foco e some quando a Lia
// pega (clique ou E). O tipo (nome, categoria, modelo) vem do catalogo de itens.

#include "Entity.h"
#include "graphics/Mesh.h"

#include <string>

class Item : public Entity {
public:
    Item(const std::string& itemId, int quantity, const Mesh* mesh, float x, float groundY, float z, float phase);

    void draw(Renderer& renderer, float time) const override;

    const std::string& getItemId() const;
    int getQuantity() const;
    Vec3 getPosition() const;

    bool isCollected() const;
    void collect();

    void setHighlight(float value);

private:
    std::string m_itemId;
    int m_quantity;
    const Mesh* m_mesh;
    float m_groundY;
    float m_phase;
    bool m_collected;
    float m_highlight;
};

#endif
