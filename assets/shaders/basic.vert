// Vertex shader (GLSL 1.10 / OpenGL 2.0)
// Usa os atributos embutidos (gl_Vertex, gl_Normal, gl_Color, gl_MultiTexCoord0),
// que funcionam tanto com os vertex arrays da Mesh quanto com o modo imediato da Model.

uniform mat4 u_model;
uniform mat4 u_normalMatrix;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_time;
uniform float u_water;
uniform float u_worldUV;
uniform float u_textureScale;

varying vec3 v_worldPos;
varying vec3 v_normal;
varying vec4 v_color;
varying vec2 v_uv;

void main()
{
    vec4 world = u_model * gl_Vertex;
    vec3 normal = (u_normalMatrix * vec4(gl_Normal, 0.0)).xyz;

    if (u_water > 0.5) {
        // Ondas: soma de senoides (mesma formula de src/graphics/Water.h)
        float x = world.x;
        float z = world.z;
        float t = u_time;

        world.y += 0.060 * sin(0.35 * x + 1.1 * t)
                 + 0.050 * sin(0.27 * z - 0.9 * t + 1.3)
                 + 0.025 * sin(0.90 * (x + z) + 2.0 * t);

        float crossTerm = 0.025 * 0.90 * cos(0.90 * (x + z) + 2.0 * t);
        float dx = 0.060 * 0.35 * cos(0.35 * x + 1.1 * t) + crossTerm;
        float dz = 0.050 * 0.27 * cos(0.27 * z - 0.9 * t + 1.3) + crossTerm;

        normal = vec3(-dx, 1.0, -dz);
    }

    v_worldPos = world.xyz;
    v_normal = normal;
    v_color = gl_Color;

    if (u_worldUV > 0.5) {
        v_uv = world.xz * u_textureScale;
    }
    else {
        v_uv = gl_MultiTexCoord0.xy * u_textureScale;
    }

    gl_Position = u_projection * u_view * world;
}
