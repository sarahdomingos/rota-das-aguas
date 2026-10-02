#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

struct ModelVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct Material {
    std::string name;

    float r = 0.8f;
    float g = 0.8f;
    float b = 0.8f;
};

struct ModelTriangle {
    ModelVertex vertices[3];
    Material material;
};

class Model {
public:
    Model();

    bool load(const std::string& objPath);

    void draw() const;

    bool isLoaded() const;
    size_t getTriangleCount() const;

private:
    std::vector<ModelTriangle> m_triangles;
    bool m_loaded;

    bool loadMaterialLibrary(
        const std::string& mtlPath,
        std::vector<Material>& materials
    );

    Material* findMaterial(
        std::vector<Material>& materials,
        const std::string& name
    );

    static int resolveIndex(int index, int size);

    void normalizeModel();
};

#endif