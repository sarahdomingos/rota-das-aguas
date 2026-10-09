#ifndef INVENTORY_H
#define INVENTORY_H

// Inventario (mochila da Lia): soma e retira quantidades de itens pelo id.
// Retirar e necessario para as entregas e para a subtracao dos desafios.

#include <string>
#include <utility>
#include <vector>

class Inventory {
public:
    void add(const std::string& itemId, int quantity);

    // Retira a quantidade se houver o suficiente; retorna false (sem mudar nada) se nao houver.
    bool remove(const std::string& itemId, int quantity);

    int count(const std::string& itemId) const;
    bool empty() const;

    // Itens na ordem em que foram pegos pela primeira vez (so os com quantidade > 0)
    std::vector<std::pair<std::string, int>> entries() const;

private:
    std::vector<std::pair<std::string, int>> m_items;
};

#endif
