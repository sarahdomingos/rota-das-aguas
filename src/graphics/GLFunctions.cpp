#include "GLFunctions.h"

#include <iostream>

namespace GL2 {

CreateShaderFn CreateShader = nullptr;
ShaderSourceFn ShaderSource = nullptr;
CompileShaderFn CompileShader = nullptr;
GetShaderivFn GetShaderiv = nullptr;
GetShaderInfoLogFn GetShaderInfoLog = nullptr;
DeleteShaderFn DeleteShader = nullptr;
CreateProgramFn CreateProgram = nullptr;
AttachShaderFn AttachShader = nullptr;
LinkProgramFn LinkProgram = nullptr;
GetProgramivFn GetProgramiv = nullptr;
GetProgramInfoLogFn GetProgramInfoLog = nullptr;
DeleteProgramFn DeleteProgram = nullptr;
UseProgramFn UseProgram = nullptr;
GetUniformLocationFn GetUniformLocation = nullptr;
UniformMatrix4fvFn UniformMatrix4fv = nullptr;
Uniform3fFn Uniform3f = nullptr;
Uniform4fFn Uniform4f = nullptr;

namespace {

template <typename T>
bool loadFunction(T& target, const char* name)
{
    target = reinterpret_cast<T>(glfwGetProcAddress(name));

    if (target == nullptr) {
        std::cerr << "Funcao OpenGL nao encontrada: " << name << std::endl;
        return false;
    }

    return true;
}

}

bool load()
{
    bool ok = true;

    ok &= loadFunction(CreateShader, "glCreateShader");
    ok &= loadFunction(ShaderSource, "glShaderSource");
    ok &= loadFunction(CompileShader, "glCompileShader");
    ok &= loadFunction(GetShaderiv, "glGetShaderiv");
    ok &= loadFunction(GetShaderInfoLog, "glGetShaderInfoLog");
    ok &= loadFunction(DeleteShader, "glDeleteShader");
    ok &= loadFunction(CreateProgram, "glCreateProgram");
    ok &= loadFunction(AttachShader, "glAttachShader");
    ok &= loadFunction(LinkProgram, "glLinkProgram");
    ok &= loadFunction(GetProgramiv, "glGetProgramiv");
    ok &= loadFunction(GetProgramInfoLog, "glGetProgramInfoLog");
    ok &= loadFunction(DeleteProgram, "glDeleteProgram");
    ok &= loadFunction(UseProgram, "glUseProgram");
    ok &= loadFunction(GetUniformLocation, "glGetUniformLocation");
    ok &= loadFunction(UniformMatrix4fv, "glUniformMatrix4fv");
    ok &= loadFunction(Uniform3f, "glUniform3f");
    ok &= loadFunction(Uniform4f, "glUniform4f");

    return ok;
}

}
