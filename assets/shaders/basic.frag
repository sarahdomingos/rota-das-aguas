// Fragment shader basico (GLSL 1.10 / OpenGL 2.0)
// Ainda sem iluminacao: apenas um sombreamento fixo pela normal
// para que as faces dos objetos fiquem distinguiveis.

uniform vec3 u_shadeDirection;

varying vec4 v_color;
varying vec3 v_normal;

void main()
{
    float shade = 1.0;

    if (length(v_normal) > 0.0001) {
        float facing = max(dot(normalize(v_normal), normalize(u_shadeDirection)), 0.0);
        shade = 0.6 + 0.4 * facing;
    }

    gl_FragColor = vec4(v_color.rgb * shade, v_color.a);
}
