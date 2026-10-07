// Fragment shader (GLSL 1.10 / OpenGL 2.0)
// Iluminacao de Phong (Blinn-Phong) por pixel: luz do sol (difusa + especular)
// e luz ambiente hemisferica (ceu/chao), com neblina e efeitos da agua.

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_useTexture;

uniform vec3 u_cameraPos;
uniform vec3 u_sunDir;
uniform vec3 u_sunColor;
uniform vec3 u_skyAmbient;
uniform vec3 u_groundAmbient;
uniform vec3 u_fogColor;
uniform float u_fogDensity;
uniform float u_useFog;

uniform float u_specular;
uniform float u_shininess;
uniform float u_highlight;
uniform float u_unlit;
uniform float u_twoSided;
uniform float u_water;
uniform float u_time;

// Mascara do casco do barco (a agua nao aparece dentro dele)
uniform mat4 u_boatInverse;
uniform float u_boatMask;

varying vec3 v_worldPos;
varying vec3 v_normal;
varying vec4 v_color;
varying vec2 v_uv;

void main()
{
    if (u_water > 0.5 && u_boatMask > 0.5) {
        // Ponto da agua nas coordenadas do barco: se estiver dentro do contorno
        // do casco (vista de cima), descarta. Assim a agua nunca "entra" no barco.
        vec3 local = (u_boatInverse * vec4(v_worldPos, 1.0)).xyz;
        // Contorno do casco na linha d'agua, medido no modelo normalizado:
        // meia largura 0.31 no meio e casco ate |z| = 0.76 (as pontas ficam fora d'agua).
        float along = local.z / 0.76;
        float along2 = along * along;
        if (abs(along) < 1.0 && abs(local.x) < 0.31 * sqrt(1.0 - along2 * along2)) {
            discard;
        }
    }

    vec4 base = v_color * u_color;

    if (u_useTexture > 0.5) {
        base.rgb *= texture2D(u_texture, v_uv).rgb;
    }

    if (u_unlit > 0.5) {
        gl_FragColor = base;
        return;
    }

    vec3 n = normalize(v_normal);
    vec3 toCamera = normalize(u_cameraPos - v_worldPos);

    if (u_twoSided > 0.5 && dot(n, toCamera) < 0.0) {
        n = -n;
    }

    vec3 l = normalize(u_sunDir);
    float diffuse = max(dot(n, l), 0.0);

    // Luz ambiente: mistura a cor do ceu (normais para cima) e do chao (para baixo)
    vec3 ambient = mix(u_groundAmbient, u_skyAmbient, 0.5 + 0.5 * n.y);

    float specular = 0.0;
    if (diffuse > 0.0) {
        vec3 halfway = normalize(l + toCamera);
        specular = pow(max(dot(n, halfway), 0.0), u_shininess) * u_specular;
    }

    vec3 color = base.rgb * (ambient + u_sunColor * diffuse) + u_sunColor * specular;
    float alpha = base.a;

    if (u_water > 0.5) {
        // Efeito Fresnel: olhando de lado, a agua reflete mais o ceu e fica menos transparente
        float fresnel = pow(1.0 - max(dot(n, toCamera), 0.0), 3.0);
        color = mix(color, u_fogColor * 1.05, fresnel * 0.6);
        alpha = mix(0.70, 0.96, fresnel);
    }

    color += vec3(1.0, 0.85, 0.45) * u_highlight;

    if (u_useFog > 0.5) {
        float dist = length(u_cameraPos - v_worldPos);
        float fog = exp(-(u_fogDensity * dist) * (u_fogDensity * dist));
        color = mix(u_fogColor, color, clamp(fog, 0.0, 1.0));
    }

    gl_FragColor = vec4(color, alpha);
}
