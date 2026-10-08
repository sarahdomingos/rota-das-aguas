#include "Items.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

std::string trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& line, char separator)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;
    while (std::getline(stream, part, separator)) {
        parts.push_back(trim(part));
    }
    return parts;
}

}

void ItemCatalog::add(const ItemDefinition& definition)
{
    m_items[definition.id] = definition;
}

const ItemDefinition* ItemCatalog::find(const std::string& id) const
{
    std::map<std::string, ItemDefinition>::const_iterator found = m_items.find(id);
    return found == m_items.end() ? nullptr : &found->second;
}

std::string ItemCatalog::nameOf(const std::string& id) const
{
    const ItemDefinition* definition = find(id);
    return definition != nullptr ? definition->name : id;
}

bool loadItemsFile(const std::string& path, ItemCatalog& catalog, std::vector<ItemPlacement>& placements)
{
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir os itens: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line = line.substr(3);
        if (line.empty() || line[0] == '#') continue;

        std::vector<std::string> parts = split(line, '|');

        if (parts[0] == "tipo" && parts.size() >= 6) {
            ItemDefinition definition;
            definition.id = parts[1];
            definition.name = parts[2];
            definition.category = parts[3];
            definition.model = parts[4];
            definition.color = { 1.0f, 1.0f, 1.0f };
            std::sscanf(parts[5].c_str(), "%f %f %f", &definition.color.x, &definition.color.y, &definition.color.z);
            catalog.add(definition);
        }
        else if (parts[0] == "item" && parts.size() >= 4) {
            ItemPlacement placement;
            placement.itemId = parts[1];
            placement.x = static_cast<float>(std::atof(parts[2].c_str()));
            placement.z = static_cast<float>(std::atof(parts[3].c_str()));
            placement.quantity = parts.size() >= 5 ? std::atoi(parts[4].c_str()) : 1;
            if (placement.quantity < 1) placement.quantity = 1;
            placements.push_back(placement);
        }
        else {
            std::cerr << "Linha de item ignorada: " << line << std::endl;
        }
    }

    return true;
}
