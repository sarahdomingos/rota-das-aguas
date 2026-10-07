#include "AssetPath.h"

#include <fstream>

namespace AssetPath {

std::string resolve(const std::string& relativePath)
{
    const char* prefixes[] = { "", "../", "../../" };

    for (const char* prefix : prefixes) {
        std::string candidate = std::string(prefix) + relativePath;

        if (std::ifstream(candidate.c_str()).good()) {
            return candidate;
        }
    }

    return relativePath;
}

}
