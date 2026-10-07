#include "ProceduralTextures.h"

#include <cmath>
#include <cstdint>

namespace ProceduralTextures {

namespace {

const float PI = 3.14159265358979323846f;

// Numero pseudoaleatorio deterministico em [0, 1) para um ponto inteiro.
float hash(int x, int y, int seed)
{
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u +
                 static_cast<uint32_t>(seed) * 2147483647u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / static_cast<float>(0x1000000);
}

float smooth(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

// Ruido de valor repetivel com periodo "period" celulas ao longo da textura.
float valueNoise(float u, float v, int period, int seed)
{
    float x = u * period;
    float y = v * period;

    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    float fx = smooth(x - x0);
    float fy = smooth(y - y0);

    auto at = [&](int ix, int iy) {
        return hash(((ix % period) + period) % period, ((iy % period) + period) % period, seed);
    };

    float a = at(x0, y0), b = at(x0 + 1, y0);
    float c = at(x0, y0 + 1), d = at(x0 + 1, y0 + 1);

    return (a + (b - a) * fx) + ((c + (d - c) * fx) - (a + (b - a) * fx)) * fy;
}

// Soma de oitavas (fractal) em [0, 1].
float fbm(float u, float v, int basePeriod, int octaves, int seed)
{
    float sum = 0.0f;
    float amplitude = 0.5f;
    float total = 0.0f;
    int period = basePeriod;

    for (int i = 0; i < octaves; ++i) {
        sum += valueNoise(u, v, period, seed + i * 17) * amplitude;
        total += amplitude;
        amplitude *= 0.5f;
        period *= 2;
    }

    return sum / total;
}

unsigned char toByte(float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    return static_cast<unsigned char>(value * 255.0f + 0.5f);
}

template <typename F>
std::vector<unsigned char> generate(F colorAt)
{
    std::vector<unsigned char> pixels(SIZE * SIZE * 3);

    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            float u = static_cast<float>(x) / SIZE;
            float v = static_cast<float>(y) / SIZE;
            float rgb[3];
            colorAt(u, v, x, y, rgb);

            unsigned char* out = &pixels[(y * SIZE + x) * 3];
            out[0] = toByte(rgb[0]);
            out[1] = toByte(rgb[1]);
            out[2] = toByte(rgb[2]);
        }
    }

    return pixels;
}

}

std::vector<unsigned char> sand()
{
    return generate([](float u, float v, int x, int y, float* rgb) {
        float dunes = fbm(u, v, 4, 3, 11);
        float grain = hash(x, y, 5);
        float speck = hash(x, y, 9) > 0.985f ? -0.18f : 0.0f;

        float value = 0.86f + 0.08f * (dunes - 0.5f) + 0.10f * (grain - 0.5f) + speck;
        rgb[0] = value;
        rgb[1] = value * 0.98f;
        rgb[2] = value * 0.95f;
    });
}

std::vector<unsigned char> wood()
{
    return generate([](float u, float v, int x, int y, float* rgb) {
        // Veios: faixas ao longo de v, deformadas por ruido
        float warp = fbm(u, v, 4, 3, 21) * 2.5f;
        float rings = 0.5f + 0.5f * std::sin((u * 14.0f + warp) * 2.0f * PI);
        float fibers = valueNoise(u * 1.0f, v * 0.05f, 64, 23);
        float grain = hash(x, y, 27);

        float value = 0.72f + 0.16f * rings + 0.08f * fibers + 0.04f * (grain - 0.5f);
        rgb[0] = value;
        rgb[1] = value * 0.93f;
        rgb[2] = value * 0.85f;
    });
}

std::vector<unsigned char> water()
{
    return generate([](float u, float v, int, int, float* rgb) {
        // Linhas claras parecidas com reflexos (ruido "ridged")
        float n = fbm(u, v, 4, 4, 31);
        float ridge = 1.0f - std::fabs(n * 2.0f - 1.0f);
        float sparkle = std::pow(ridge, 6.0f);

        float value = 0.80f + 0.20f * sparkle;
        rgb[0] = value;
        rgb[1] = value;
        rgb[2] = value;
    });
}

std::vector<unsigned char> plaster()
{
    return generate([](float u, float v, int x, int y, float* rgb) {
        float stains = fbm(u, v, 4, 4, 41);
        float grain = hash(x, y, 43);

        float value = 0.88f + 0.10f * (stains - 0.5f) + 0.06f * (grain - 0.5f);
        rgb[0] = value;
        rgb[1] = value;
        rgb[2] = value;
    });
}

std::vector<unsigned char> stone()
{
    return generate([](float u, float v, int x, int y, float* rgb) {
        // Paralelepipedos: celulas de Voronoi repetiveis com rejunte escuro
        const int cells = 6;
        float px = u * cells;
        float py = v * cells;
        int cx = static_cast<int>(std::floor(px));
        int cy = static_cast<int>(std::floor(py));

        float nearest = 10.0f;
        float second = 10.0f;
        float shade = 0.0f;

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int ix = cx + dx;
                int iy = cy + dy;
                int wx = ((ix % cells) + cells) % cells;
                int wy = ((iy % cells) + cells) % cells;

                float fx = ix + 0.2f + 0.6f * hash(wx, wy, 51);
                float fy = iy + 0.2f + 0.6f * hash(wx, wy, 53);
                float d = std::sqrt((px - fx) * (px - fx) + (py - fy) * (py - fy));

                if (d < nearest) {
                    second = nearest;
                    nearest = d;
                    shade = hash(wx, wy, 57);
                }
                else if (d < second) {
                    second = d;
                }
            }
        }

        float edge = second - nearest;
        float mortar = edge < 0.08f ? 0.55f : 1.0f;
        float grain = hash(x, y, 59);

        float value = (0.78f + 0.14f * shade + 0.05f * (grain - 0.5f)) * mortar;
        rgb[0] = value;
        rgb[1] = value * 0.98f;
        rgb[2] = value * 0.96f;
    });
}

}
