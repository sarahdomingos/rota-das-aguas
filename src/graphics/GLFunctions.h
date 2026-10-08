#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

// Carregamento manual das funcoes de shader do OpenGL 2.0.
// No Windows, a opengl32.dll so exporta o OpenGL 1.1; o resto precisa
// ser buscado em tempo de execucao (glfwGetProcAddress). Fazemos isso
// aqui para nao depender de GLAD/GLEW.

#include <GLFW/glfw3.h>

typedef char GLchar;

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

namespace GL2 {

typedef GLuint (APIENTRY *CreateShaderFn)(GLenum type);
typedef void (APIENTRY *ShaderSourceFn)(GLuint shader, GLsizei count, const GLchar* const* source, const GLint* length);
typedef void (APIENTRY *CompileShaderFn)(GLuint shader);
typedef void (APIENTRY *GetShaderivFn)(GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRY *GetShaderInfoLogFn)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY *DeleteShaderFn)(GLuint shader);
typedef GLuint (APIENTRY *CreateProgramFn)();
typedef void (APIENTRY *AttachShaderFn)(GLuint program, GLuint shader);
typedef void (APIENTRY *LinkProgramFn)(GLuint program);
typedef void (APIENTRY *GetProgramivFn)(GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRY *GetProgramInfoLogFn)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRY *DeleteProgramFn)(GLuint program);
typedef void (APIENTRY *UseProgramFn)(GLuint program);
typedef GLint (APIENTRY *GetUniformLocationFn)(GLuint program, const GLchar* name);
typedef void (APIENTRY *UniformMatrix4fvFn)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRY *Uniform3fFn)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY *Uniform4fFn)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY *Uniform1fFn)(GLint location, GLfloat v0);
typedef void (APIENTRY *Uniform1iFn)(GLint location, GLint v0);

extern CreateShaderFn CreateShader;
extern ShaderSourceFn ShaderSource;
extern CompileShaderFn CompileShader;
extern GetShaderivFn GetShaderiv;
extern GetShaderInfoLogFn GetShaderInfoLog;
extern DeleteShaderFn DeleteShader;
extern CreateProgramFn CreateProgram;
extern AttachShaderFn AttachShader;
extern LinkProgramFn LinkProgram;
extern GetProgramivFn GetProgramiv;
extern GetProgramInfoLogFn GetProgramInfoLog;
extern DeleteProgramFn DeleteProgram;
extern UseProgramFn UseProgram;
extern GetUniformLocationFn GetUniformLocation;
extern UniformMatrix4fvFn UniformMatrix4fv;
extern Uniform3fFn Uniform3f;
extern Uniform4fFn Uniform4f;
extern Uniform1fFn Uniform1f;
extern Uniform1iFn Uniform1i;

// Precisa ser chamada depois que o contexto OpenGL estiver ativo.
bool load();

}

#endif
