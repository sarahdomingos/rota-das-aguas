#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

// Texto na tela (interface 2D). A fonte e desenhada uma vez pelo proprio
// Windows (GDI) num atlas de caracteres Latin-1, transformado em textura, o
// que cobre todos os acentos do portugues (a, a, c, e, e, i, o, o, u...).
// O texto e desenhado por cima da cena, em pixels, com origem no canto
// superior esquerdo. Sera reaproveitado por dialogos, HUD e inventario.
//
// Uso a cada quadro:
//   text.begin(renderer, largura, altura);
//   text.drawBox(...); text.drawText(...);
//   text.end();

#include "graphics/Texture.h"

#include <string>
#include <vector>

class Renderer;

struct Color {
    float r;
    float g;
    float b;
    float a;
};

class TextRenderer {
public:
    TextRenderer();

    // pixelHeight: altura da fonte no atlas (o texto pode ser desenhado em outras escalas)
    bool init(int pixelHeight = 32, const char* fontName = "Segoe UI");

    void begin(Renderer& renderer, int screenWidth, int screenHeight);
    void end();

    // Desenha uma linha de texto (UTF-8); (x, y) e o canto superior esquerdo.
    void drawText(float x, float y, const std::string& text, float scale, const Color& color);

    // Texto quebrado em linhas para caber em maxWidth; retorna a altura usada.
    float drawWrapped(float x, float y, float maxWidth, const std::string& text, float scale, const Color& color);

    // Retangulo solido (fundo de caixa de dialogo, barras do HUD...).
    void drawBox(float x, float y, float width, float height, const Color& color);

    float measure(const std::string& text, float scale) const;
    float lineHeight(float scale) const;

    // Quebra o texto em linhas que cabem em maxWidth.
    std::vector<std::string> wrap(const std::string& text, float scale, float maxWidth) const;

private:
    struct Glyph {
        float u0, v0, u1, v1;
        float width;    // largura da celula desenhada (pixels do atlas)
        float advance;  // avanco do cursor (pixels do atlas)
    };

    Texture m_atlas;
    Glyph m_glyphs[256];
    float m_cellHeight;
    bool m_ready;

    Renderer* m_renderer;

    static std::vector<unsigned int> decodeUtf8(const std::string& text);
    const Glyph& glyphFor(unsigned int codepoint) const;
};

#endif
