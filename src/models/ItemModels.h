#ifndef ITEM_MODELS_H
#define ITEM_MODELS_H

// Modelos simples dos itens coletaveis, gerados por codigo (base em y = 0).
// O nome do modelo vem do arquivo de itens (coluna "modelo").

#include "graphics/Mesh.h"

#include <string>

namespace ItemModels {

// Modelos: "coco", "concha", "fita" (outros nomes viram uma caixinha).
Mesh build(const std::string& model, const Vec3& color);

}

#endif
