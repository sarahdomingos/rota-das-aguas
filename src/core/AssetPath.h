#ifndef ASSET_PATH_H
#define ASSET_PATH_H

#include <string>

namespace AssetPath {

// Procura o arquivo a partir da pasta atual e de ate duas pastas acima,
// para funcionar tanto rodando da raiz (build.bat) quanto de build/ (CMake).
std::string resolve(const std::string& relativePath);

}

#endif
