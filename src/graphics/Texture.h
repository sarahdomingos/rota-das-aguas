#ifndef TEXTURE_H
#define TEXTURE_H

// Textura 2D do OpenGL, criada a partir de pixels RGB (gerados por codigo)
// ou de um arquivo .bmp de 24 bits.

#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // pixels: width * height * 3 bytes (RGB), linha de baixo primeiro.
    bool create(int width, int height, const std::vector<unsigned char>& pixels);
    bool loadBMP(const std::string& path);

    void bind() const;
    bool isValid() const;

private:
    GLuint m_id;
};

#endif
