#include "Model.h"

#include <GL/gl.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>

namespace {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec2 {
    float u;
    float v;
};

struct FaceIndex {
    int vertex;
    int texcoord;
    int normal;
};

Vec3 subtract(const Vec3& a, const Vec3& b)
{
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

void normalize(Vec3& v)
{
    float length = std::sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );

    if (length > 0.000001f) {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }
}

FaceIndex parseFaceIndex(const std::string& token)
{
    FaceIndex result{0, 0, 0};

    std::stringstream ss(token);
    std::string part;
    int index = 0;

    while (std::getline(ss, part, '/')) {

        if (!part.empty()) {
            int value = std::stoi(part);

            if (index == 0) {
                result.vertex = value;
            }
            else if (index == 1) {
                result.texcoord = value;
            }
            else if (index == 2) {
                result.normal = value;
            }
        }

        index++;
    }

    return result;
}

}

Model::Model()
    : m_loaded(false)
{
}

bool Model::load(const std::string& objPath)
{
    std::ifstream file(objPath);

    if (!file.is_open()) {
        std::cerr
            << "[Model] Nao foi possivel abrir: "
            << objPath
            << std::endl;

        return false;
    }

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> texcoords;

    std::vector<ModelMaterial> materials;

    ModelMaterial defaultMaterial;
    defaultMaterial.name = "default";
    materials.push_back(defaultMaterial);

    ModelMaterial* currentMaterial = &materials[0];

    std::string line;

    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);

        std::string command;
        ss >> command;

        // ------------------------------------------
        // Vertice
        // ------------------------------------------

        if (command == "v") {

            Vec3 position;

            ss >> position.x
               >> position.y
               >> position.z;

            positions.push_back(position);
        }

        // ------------------------------------------
        // Normal
        // ------------------------------------------

        else if (command == "vn") {

            Vec3 normal;

            ss >> normal.x
               >> normal.y
               >> normal.z;

            normals.push_back(normal);
        }

        // ------------------------------------------
        // Coordenada UV
        // ------------------------------------------

        else if (command == "vt") {

            Vec2 uv;

            ss >> uv.u
               >> uv.v;

            texcoords.push_back(uv);
        }

        // ------------------------------------------
        // ModelMaterial library
        // ------------------------------------------

        else if (command == "mtllib") {

            std::string mtlFile;
            ss >> mtlFile;

            // Descobre a pasta onde o OBJ está.
            size_t lastSlash = objPath.find_last_of("/\\");

            std::string directory;

            if (lastSlash != std::string::npos) {
                directory = objPath.substr(0, lastSlash + 1);
            }

            std::string mtlPath = directory + mtlFile;

            loadMaterialLibrary(mtlPath, materials);

            currentMaterial = &materials[0];
        }

        // ------------------------------------------
        // ModelMaterial utilizado
        // ------------------------------------------

        else if (command == "usemtl") {

            std::string materialName;
            ss >> materialName;

            ModelMaterial* material =
                findMaterial(materials, materialName);

            if (material != nullptr) {
                currentMaterial = material;
            }
        }

        // ------------------------------------------
        // Face
        // ------------------------------------------

        else if (command == "f") {

            std::vector<FaceIndex> face;

            std::string token;

            while (ss >> token) {
                face.push_back(parseFaceIndex(token));
            }

            if (face.size() < 3) {
                continue;
            }

            // Fan triangulation:
            //
            //      0
            //     / \
            //    /   \
            //   1-----2
            //
            // Para um polígono maior:
            //
            // 0-1-2
            // 0-2-3
            // 0-3-4
            // ...

            for (size_t i = 1; i + 1 < face.size(); ++i) {

                FaceIndex indices[3] = {
                    face[0],
                    face[i],
                    face[i + 1]
                };

                ModelTriangle triangle;

                bool hasNormals = true;

                for (int j = 0; j < 3; ++j) {

                    int vertexIndex =
                        resolveIndex(
                            indices[j].vertex,
                            static_cast<int>(positions.size())
                        );

                    if (vertexIndex < 0 ||
                        vertexIndex >= static_cast<int>(positions.size())) {

                        std::cerr
                            << "[Model] Indice de vertice invalido."
                            << std::endl;

                        return false;
                    }

                    Vec3 position =
                        positions[vertexIndex];

                    triangle.vertices[j].x = position.x;
                    triangle.vertices[j].y = position.y;
                    triangle.vertices[j].z = position.z;

                    triangle.vertices[j].nx = 0.0f;
                    triangle.vertices[j].ny = 0.0f;
                    triangle.vertices[j].nz = 0.0f;

                    triangle.vertices[j].u = 0.0f;
                    triangle.vertices[j].v = 0.0f;

                    // Normal
                    if (indices[j].normal != 0) {

                        int normalIndex =
                            resolveIndex(
                                indices[j].normal,
                                static_cast<int>(normals.size())
                            );

                        if (normalIndex >= 0 &&
                            normalIndex < static_cast<int>(normals.size())) {

                            triangle.vertices[j].nx =
                                normals[normalIndex].x;

                            triangle.vertices[j].ny =
                                normals[normalIndex].y;

                            triangle.vertices[j].nz =
                                normals[normalIndex].z;
                        }
                    }
                    else {
                        hasNormals = false;
                    }

                    // UV
                    if (indices[j].texcoord != 0) {

                        int uvIndex =
                            resolveIndex(
                                indices[j].texcoord,
                                static_cast<int>(texcoords.size())
                            );

                        if (uvIndex >= 0 &&
                            uvIndex < static_cast<int>(texcoords.size())) {

                            triangle.vertices[j].u =
                                texcoords[uvIndex].u;

                            triangle.vertices[j].v =
                                texcoords[uvIndex].v;
                        }
                    }
                }

                // ------------------------------------------
                // Se o OBJ não tiver normais,
                // calcula uma normal para a face.
                // ------------------------------------------

                if (!hasNormals) {

                    Vec3 a{
                        triangle.vertices[0].x,
                        triangle.vertices[0].y,
                        triangle.vertices[0].z
                    };

                    Vec3 b{
                        triangle.vertices[1].x,
                        triangle.vertices[1].y,
                        triangle.vertices[1].z
                    };

                    Vec3 c{
                        triangle.vertices[2].x,
                        triangle.vertices[2].y,
                        triangle.vertices[2].z
                    };

                    Vec3 edge1 = subtract(b, a);
                    Vec3 edge2 = subtract(c, a);

                    Vec3 normal = cross(edge1, edge2);

                    normalize(normal);

                    for (int j = 0; j < 3; ++j) {
                        triangle.vertices[j].nx = normal.x;
                        triangle.vertices[j].ny = normal.y;
                        triangle.vertices[j].nz = normal.z;
                    }
                }

                triangle.material = *currentMaterial;

                m_triangles.push_back(triangle);
            }
        }
    }

    file.close();

    if (m_triangles.empty()) {

        std::cerr
            << "[Model] Nenhum triangulo encontrado em: "
            << objPath
            << std::endl;

        return false;
    }

    normalizeModel();

    m_loaded = true;

    std::cout
        << "[Model] Modelo carregado: "
        << objPath
        << std::endl;

    std::cout
        << "[Model] Triangulos: "
        << m_triangles.size()
        << std::endl;

    return true;
}

bool Model::loadMaterialLibrary(
    const std::string& mtlPath,
    std::vector<ModelMaterial>& materials)
{
    std::ifstream file(mtlPath);

    if (!file.is_open()) {

        std::cerr
            << "[Model] Aviso: nao foi possivel abrir MTL: "
            << mtlPath
            << std::endl;

        return false;
    }

    ModelMaterial* current = nullptr;

    std::string line;

    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);

        std::string command;
        ss >> command;

        if (command == "newmtl") {

            std::string name;
            ss >> name;

            ModelMaterial material;
            material.name = name;

            materials.push_back(material);

            current = &materials.back();
        }

        else if (command == "Kd" && current != nullptr) {

            ss >> current->r
               >> current->g
               >> current->b;
        }
    }

    file.close();

    return true;
}

ModelMaterial* Model::findMaterial(
    std::vector<ModelMaterial>& materials,
    const std::string& name)
{
    for (auto& material : materials) {

        if (material.name == name) {
            return &material;
        }
    }

    return nullptr;
}

int Model::resolveIndex(int index, int size)
{
    if (index > 0) {
        return index - 1;
    }

    if (index < 0) {
        return size + index;
    }

    return -1;
}

void Model::normalizeModel()
{
    if (m_triangles.empty()) {
        return;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& triangle : m_triangles) {

        for (const auto& vertex : triangle.vertices) {

            minX = std::min(minX, vertex.x);
            minY = std::min(minY, vertex.y);
            minZ = std::min(minZ, vertex.z);

            maxX = std::max(maxX, vertex.x);
            maxY = std::max(maxY, vertex.y);
            maxZ = std::max(maxZ, vertex.z);
        }
    }

    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;

    float largestDimension =
        std::max(sizeX, std::max(sizeY, sizeZ));

    if (largestDimension <= 0.000001f) {
        return;
    }

    // Faz o maior eixo ter aproximadamente 2 unidades.
    float scale = 2.0f / largestDimension;

    for (auto& triangle : m_triangles) {

        for (auto& vertex : triangle.vertices) {

            vertex.x =
                (vertex.x - centerX) * scale;

            vertex.y =
                (vertex.y - centerY) * scale;

            vertex.z =
                (vertex.z - centerZ) * scale;
        }
    }
}

void Model::draw() const
{
    if (!m_loaded) {
        return;
    }

    glBegin(GL_TRIANGLES);

    for (const auto& triangle : m_triangles) {

        glColor3f(
            triangle.material.r,
            triangle.material.g,
            triangle.material.b
        );

        for (int i = 0; i < 3; ++i) {

            const ModelVertex& vertex =
                triangle.vertices[i];

            glNormal3f(
                vertex.nx,
                vertex.ny,
                vertex.nz
            );

            glTexCoord2f(
                vertex.u,
                vertex.v
            );

            glVertex3f(
                vertex.x,
                vertex.y,
                vertex.z
            );
        }
    }

    glEnd();
}

bool Model::isLoaded() const
{
    return m_loaded;
}

size_t Model::getTriangleCount() const
{
    return m_triangles.size();
}