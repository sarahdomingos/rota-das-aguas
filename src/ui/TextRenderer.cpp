#include <windows.h>

#include "TextRenderer.h"

#include "graphics/Renderer.h"

#include <cstring>

namespace {

const int FIRST_CHAR = 32;
const int COLUMNS = 16;
const int ROWS = 14; // caracteres 32 a 255

int nextPowerOfTwo(int value)
{
    int result = 1;
    while (result < value) result *= 2;
    return result;
}

}

TextRenderer::TextRenderer()
    : m_cellHeight(0.0f),
      m_ready(false),
      m_renderer(nullptr)
{
    std::memset(m_glyphs, 0, sizeof(m_glyphs));
}

bool TextRenderer::init(int pixelHeight, const char* fontName)
{
    HDC dc = CreateCompatibleDC(nullptr);
    HFONT font = CreateFontA(-pixelHeight, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, fontName);
    HGDIOBJ oldFont = SelectObject(dc, font);

    TEXTMETRICA metrics;
    GetTextMetricsA(dc, &metrics);

    int cellWidth = metrics.tmMaxCharWidth + 4;
    int cellHeight = metrics.tmHeight + 2;
    // Espaco vazio entre as celulas evita que a filtragem "puxe" pixels do caractere vizinho
    const int gap = 4;
    int strideX = cellWidth + gap;
    int strideY = cellHeight + gap;
    int atlasWidth = nextPowerOfTwo(strideX * COLUMNS);
    int atlasHeight = nextPowerOfTwo(strideY * ROWS);

    // Bitmap de 32 bits de cima para baixo, onde o Windows desenha os caracteres
    BITMAPINFO info;
    std::memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = atlasWidth;
    info.bmiHeader.biHeight = -atlasHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap == nullptr || bits == nullptr) {
        SelectObject(dc, oldFont);
        DeleteObject(font);
        DeleteDC(dc);
        return false;
    }

    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    std::memset(bits, 0, atlasWidth * atlasHeight * 4);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));

    for (int code = FIRST_CHAR; code < 256; ++code) {
        // 0x7F a 0x9F nao sao caracteres visiveis no Latin-1
        if (code >= 0x7F && code < 0xA0) continue;

        int index = code - FIRST_CHAR;
        int x = (index % COLUMNS) * strideX;
        int y = (index / COLUMNS) * strideY;

        wchar_t character = static_cast<wchar_t>(code); // Latin-1 = mesmos codigos do Unicode
        TextOutW(dc, x + 2, y + 1, &character, 1);

        SIZE size;
        GetTextExtentPoint32W(dc, &character, 1, &size);

        Glyph& glyph = m_glyphs[code];
        glyph.u0 = static_cast<float>(x) / atlasWidth;
        glyph.v0 = static_cast<float>(y) / atlasHeight;
        glyph.u1 = static_cast<float>(x + cellWidth) / atlasWidth;
        glyph.v1 = static_cast<float>(y + cellHeight) / atlasHeight;
        glyph.width = static_cast<float>(cellWidth);
        glyph.advance = static_cast<float>(size.cx);
    }

    GdiFlush();

    // Cor branca; a transparencia vem da cobertura do caractere
    std::vector<unsigned char> pixels(atlasWidth * atlasHeight * 4);
    const unsigned char* source = static_cast<const unsigned char*>(bits);
    for (int i = 0; i < atlasWidth * atlasHeight; ++i) {
        pixels[i * 4 + 0] = 255;
        pixels[i * 4 + 1] = 255;
        pixels[i * 4 + 2] = 255;
        pixels[i * 4 + 3] = source[i * 4 + 1]; // canal verde (texto em tons de cinza)
    }

    SelectObject(dc, oldBitmap);
    SelectObject(dc, oldFont);
    DeleteObject(bitmap);
    DeleteObject(font);
    DeleteDC(dc);

    m_cellHeight = static_cast<float>(cellHeight);
    m_ready = m_atlas.createRGBA(atlasWidth, atlasHeight, pixels);
    return m_ready;
}

std::vector<unsigned int> TextRenderer::decodeUtf8(const std::string& text)
{
    std::vector<unsigned int> result;

    for (size_t i = 0; i < text.size();) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        unsigned int codepoint = '?';
        int length = 1;

        if (c < 0x80) {
            codepoint = c;
        }
        else if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
            codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3F);
            length = 2;
        }
        else if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
            codepoint = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) |
                        (static_cast<unsigned char>(text[i + 2]) & 0x3F);
            length = 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            length = 4;
        }

        result.push_back(codepoint);
        i += length;
    }

    return result;
}

const TextRenderer::Glyph& TextRenderer::glyphFor(unsigned int codepoint) const
{
    // Travessao e aspas curvas viram os equivalentes simples; o resto fora do Latin-1 vira "?"
    if (codepoint == 0x2013 || codepoint == 0x2014) codepoint = '-';
    if (codepoint == 0x2018 || codepoint == 0x2019) codepoint = '\'';
    if (codepoint == 0x201C || codepoint == 0x201D) codepoint = '"';

    if (codepoint < static_cast<unsigned int>(FIRST_CHAR) || codepoint > 255 ||
        (codepoint >= 0x7F && codepoint < 0xA0)) {
        codepoint = '?';
    }

    return m_glyphs[codepoint];
}

void TextRenderer::begin(Renderer& renderer, int screenWidth, int screenHeight)
{
    m_renderer = &renderer;
    renderer.begin2D(screenWidth, screenHeight);
}

void TextRenderer::end()
{
    if (m_renderer != nullptr) {
        m_renderer->end2D();
        m_renderer = nullptr;
    }
}

void TextRenderer::drawBox(float x, float y, float width, float height, const Color& color)
{
    if (m_renderer == nullptr) return;

    m_renderer->setMaterial2D(nullptr);

    glBegin(GL_QUADS);
    glColor4f(color.r, color.g, color.b, color.a);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void TextRenderer::drawText(float x, float y, const std::string& text, float scale, const Color& color)
{
    if (m_renderer == nullptr || !m_ready) return;

    m_renderer->setMaterial2D(&m_atlas);

    float cursor = x;
    float height = m_cellHeight * scale;

    glBegin(GL_QUADS);
    glColor4f(color.r, color.g, color.b, color.a);

    for (unsigned int codepoint : decodeUtf8(text)) {
        const Glyph& glyph = glyphFor(codepoint);
        float width = glyph.width * scale;

        glTexCoord2f(glyph.u0, glyph.v0); glVertex2f(cursor, y);
        glTexCoord2f(glyph.u1, glyph.v0); glVertex2f(cursor + width, y);
        glTexCoord2f(glyph.u1, glyph.v1); glVertex2f(cursor + width, y + height);
        glTexCoord2f(glyph.u0, glyph.v1); glVertex2f(cursor, y + height);

        cursor += glyph.advance * scale;
    }

    glEnd();
}

float TextRenderer::measure(const std::string& text, float scale) const
{
    float width = 0.0f;
    for (unsigned int codepoint : decodeUtf8(text)) {
        width += glyphFor(codepoint).advance * scale;
    }
    return width;
}

float TextRenderer::lineHeight(float scale) const
{
    return m_cellHeight * scale;
}

std::vector<std::string> TextRenderer::wrap(const std::string& text, float scale, float maxWidth) const
{
    std::vector<std::string> lines;
    std::string line;
    std::string word;

    auto flushWord = [&]() {
        if (word.empty()) return;
        std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && measure(candidate, scale) > maxWidth) {
            lines.push_back(line);
            line = word;
        }
        else {
            line = candidate;
        }
        word.clear();
    };

    for (char c : text) {
        if (c == ' ') {
            flushWord();
        }
        else if (c == '\n') {
            flushWord();
            lines.push_back(line);
            line.clear();
        }
        else {
            word += c;
        }
    }

    flushWord();
    if (!line.empty()) {
        lines.push_back(line);
    }

    return lines;
}

float TextRenderer::drawWrapped(float x, float y, float maxWidth, const std::string& text, float scale, const Color& color)
{
    std::vector<std::string> lines = wrap(text, scale, maxWidth);
    float lineStep = lineHeight(scale);

    for (size_t i = 0; i < lines.size(); ++i) {
        drawText(x, y + i * lineStep, lines[i], scale, color);
    }

    return lines.size() * lineStep;
}
