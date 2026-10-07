#include "Inventory.h"

void Inventory::add(const std::string& itemId, int quantity)
{
    if (quantity <= 0) {
        return;
    }

    for (std::pair<std::string, int>& entry : m_items) {
        if (entry.first == itemId) {
            entry.second += quantity;
            return;
        }
    }

    m_items.push_back(std::make_pair(itemId, quantity));
}

bool Inventory::remove(const std::string& itemId, int quantity)
{
    for (std::pair<std::string, int>& entry : m_items) {
        if (entry.first == itemId) {
            if (entry.second < quantity) {
                return false;
            }
            entry.second -= quantity;
            return true;
        }
    }

    return quantity <= 0;
}

int Inventory::count(const std::string& itemId) const
{
    for (const std::pair<std::string, int>& entry : m_items) {
        if (entry.first == itemId) {
            return entry.second;
        }
    }
    return 0;
}

bool Inventory::empty() const
{
    return entries().empty();
}

std::vector<std::pair<std::string, int>> Inventory::entries() const
{
    std::vector<std::pair<std::string, int>> result;
    for (const std::pair<std::string, int>& entry : m_items) {
        if (entry.second > 0) {
            result.push_back(entry);
        }
    }
    return result;
}
