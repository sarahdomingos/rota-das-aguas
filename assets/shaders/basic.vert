// Vertex shader basico (GLSL 1.10 / OpenGL 2.0)
// Usa os atributos embutidos (gl_Vertex, gl_Normal, gl_Color) para funcionar
// tanto com os vertex arrays da classe Mesh quanto com o modo imediato da Model.

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec4 u_tint;

varying vec4 v_color;
varying vec3 v_normal;

void main()
{
    v_color = gl_Color * u_tint;
    v_normal = (u_model * vec4(gl_Normal, 0.0)).xyz;

    gl_Position = u_projection * u_view * u_model * gl_Vertex;
}
