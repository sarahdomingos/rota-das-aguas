#ifndef ITEMS_H
#define ITEMS_H

// Itens do jogo, definidos como dados (assets/itens/<capitulo>.txt):
// - o catalogo diz o que cada tipo de item e (nome, categoria, modelo, cor);
// - as posicoes dizem onde cada item aparece no mundo.
// O formato esta documentado no topo de assets/itens/maceio.txt.

#include "graphics/MathUtils.h"

#include <map>
#include <string>
#include <vector>

struct ItemDefinition {
    std::string id;
    std::string name;
    std::string category;
    std::string model;      // nome do modelo gerado por codigo (models/ItemModels)
    Vec3 color;
};

struct ItemPlacement {
    std::string itemId;
    float x;
    float z;
    int quantity;
};

class ItemCatalog {
public:
    void add(const ItemDefinition& definition);
    const ItemDefinition* find(const std::string& id) const;

    // Nome para mostrar na tela (ou o proprio id, se o item nao existir)
    std::string nameOf(const std::string& id) const;

private:
    std::map<std::string, ItemDefinition> m_items;
};

// Le o arquivo de itens de um capitulo (tipos e posicoes).
bool loadItemsFile(const std::string& path, ItemCatalog& catalog, std::vector<ItemPlacement>& placements);

#endif
