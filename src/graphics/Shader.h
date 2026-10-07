#ifndef SHADER_H
#define SHADER_H

#include "GLFunctions.h"
#include "MathUtils.h"

#include <string>

// Programa GLSL (vertex + fragment) carregado de arquivos em assets/shaders.
class Shader {
public:
    Shader();
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    void use() const;

    void setMat4(const char* name, const float matrix[16]) const;
    void setVec3(const char* name, const Vec3& value) const;
    void setVec4(const char* name, float x, float y, float z, float w) const;

private:
    GLuint m_program;

    static bool readFile(const std::string& path, std::string& contents);
    static GLuint compile(GLenum type, const std::string& source, const std::string& path);
};

#endif
