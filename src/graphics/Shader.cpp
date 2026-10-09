#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Shader::Shader()
    : m_program(0)
{
}

Shader::~Shader()
{
    if (m_program != 0) {
        GL2::DeleteProgram(m_program);
    }
}

bool Shader::readFile(const std::string& path, std::string& contents)
{
    std::ifstream file(path.c_str());

    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir o shader: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    contents = buffer.str();

    return true;
}

GLuint Shader::compile(GLenum type, const std::string& source, const std::string& path)
{
    GLuint shader = GL2::CreateShader(type);

    const GLchar* sourcePointer = source.c_str();
    GL2::ShaderSource(shader, 1, &sourcePointer, nullptr);
    GL2::CompileShader(shader);

    GLint success = GL_FALSE;
    GL2::GetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE) {
        GLint logLength = 0;
        GL2::GetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength > 1 ? logLength : 1, '\0');
        GL2::GetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());

        std::cerr << "Erro ao compilar " << path << ":\n" << log.data() << std::endl;

        GL2::DeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexSource;
    std::string fragmentSource;

    if (!readFile(vertexPath, vertexSource) || !readFile(fragmentPath, fragmentSource)) {
        return false;
    }

    GLuint vertexShader = compile(GL_VERTEX_SHADER, vertexSource, vertexPath);
    GLuint fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) GL2::DeleteShader(vertexShader);
        if (fragmentShader != 0) GL2::DeleteShader(fragmentShader);
        return false;
    }

    GLuint program = GL2::CreateProgram();
    GL2::AttachShader(program, vertexShader);
    GL2::AttachShader(program, fragmentShader);
    GL2::LinkProgram(program);

    // Depois de linkados, os shaders individuais nao sao mais necessarios.
    GL2::DeleteShader(vertexShader);
    GL2::DeleteShader(fragmentShader);

    GLint success = GL_FALSE;
    GL2::GetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
        GLint logLength = 0;
        GL2::GetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<GLchar> log(logLength > 1 ? logLength : 1, '\0');
        GL2::GetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());

        std::cerr << "Erro ao linkar o programa de shader:\n" << log.data() << std::endl;

        GL2::DeleteProgram(program);
        return false;
    }

    if (m_program != 0) {
        GL2::DeleteProgram(m_program);
    }

    m_program = program;
    m_locations.clear();
    return true;
}

void Shader::use() const
{
    GL2::UseProgram(m_program);
}

GLint Shader::location(const char* name) const
{
    std::map<std::string, GLint>::const_iterator found = m_locations.find(name);

    if (found != m_locations.end()) {
        return found->second;
    }

    GLint value = GL2::GetUniformLocation(m_program, name);
    m_locations[name] = value;
    return value;
}

void Shader::setMat4(const char* name, const float matrix[16]) const
{
    GL2::UniformMatrix4fv(location(name), 1, GL_FALSE, matrix);
}

void Shader::setVec3(const char* name, const Vec3& value) const
{
    GL2::Uniform3f(location(name), value.x, value.y, value.z);
}

void Shader::setVec4(const char* name, float x, float y, float z, float w) const
{
    GL2::Uniform4f(location(name), x, y, z, w);
}

void Shader::setFloat(const char* name, float value) const
{
    GL2::Uniform1f(location(name), value);
}

void Shader::setInt(const char* name, int value) const
{
    GL2::Uniform1i(location(name), value);
}
