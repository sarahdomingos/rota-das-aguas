#include <windows.h>

#include "Texture.h"

#include <GL/glu.h>

#include <cstdint>
#include <fstream>
#include <iostream>

Texture::Texture()
    : m_id(0)
{
}

Texture::~Texture()
{
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
}

bool Texture::create(int width, int height, const std::vector<unsigned char>& pixels)
{
    if (width <= 0 || height <= 0 || pixels.size() < static_cast<size_t>(width * height * 3)) {
        return false;
    }

    if (m_id == 0) {
        glGenTextures(1, &m_id);
    }

    glBindTexture(GL_TEXTURE_2D, m_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Mipmaps evitam o "chiado" da textura em superficies distantes
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    return true;
}

bool Texture::loadBMP(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir a textura: " << path << std::endl;
        return false;
    }

    unsigned char header[54];
    file.read(reinterpret_cast<char*>(header), 54);

    if (!file || header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Arquivo BMP invalido: " << path << std::endl;
        return false;
    }

    auto read32 = [&](int offset) {
        return static_cast<int32_t>(header[offset] | (header[offset + 1] << 8) |
                                    (header[offset + 2] << 16) | (header[offset + 3] << 24));
    };

    int dataOffset = read32(10);
    int width = read32(18);
    int height = read32(22);
    int bitsPerPixel = header[28] | (header[29] << 8);

    if (bitsPerPixel != 24 || width <= 0 || height == 0) {
        std::cerr << "Somente BMP de 24 bits e suportado: " << path << std::endl;
        return false;
    }

    bool bottomUp = height > 0;
    if (!bottomUp) {
        height = -height;
    }

    int rowSize = (width * 3 + 3) & ~3;
    std::vector<unsigned char> row(rowSize);
    std::vector<unsigned char> pixels(width * height * 3);

    file.seekg(dataOffset, std::ios::beg);

    for (int y = 0; y < height; ++y) {
        file.read(reinterpret_cast<char*>(row.data()), rowSize);

        // O OpenGL espera a linha de baixo primeiro, como no BMP comum.
        int targetRow = bottomUp ? y : (height - 1 - y);

        for (int x = 0; x < width; ++x) {
            unsigned char* out = &pixels[(targetRow * width + x) * 3];
            out[0] = row[x * 3 + 2]; // BGR -> RGB
            out[1] = row[x * 3 + 1];
            out[2] = row[x * 3 + 0];
        }
    }

    return create(width, height, pixels);
}

bool Texture::createRGBA(int width, int height, const std::vector<unsigned char>& pixels)
{
    if (width <= 0 || height <= 0 || pixels.size() < static_cast<size_t>(width * height * 4)) {
        return false;
    }

    if (m_id == 0) {
        glGenTextures(1, &m_id);
    }

    glBindTexture(GL_TEXTURE_2D, m_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    return true;
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}

bool Texture::isValid() const
{
    return m_id != 0;
}
