#ifndef DIALOGUE_H
#define DIALOGUE_H

// Sistema de dialogo generico, orientado a dados: as falas ficam em arquivos
// de texto em assets/dialogos (um por capitulo), em blocos identificados por
// id. O formato esta documentado no topo de assets/dialogos/maceio.txt.
//
// O Dialogue so cuida do conteudo e do avanco das falas (com o texto
// aparecendo letra por letra); quem desenha a caixa e quem troca o estado do
// jogo e o Game.

#include <map>
#include <string>
#include <vector>

struct DialogueLine {
    std::string speaker;
    std::string text;
};

class Dialogue {
public:
    Dialogue();

    // Carrega (ou acrescenta) os dialogos de um arquivo.
    bool loadFile(const std::string& path);
    bool has(const std::string& id) const;

    // Comeca o dialogo; retorna false se o id nao existir.
    bool start(const std::string& id);

    // Avanca as letras do efeito de "maquina de escrever".
    void update(float deltaTime);

    // Se a fala ainda esta aparecendo, mostra ela inteira; senao vai para a
    // proxima. Retorna false quando o dialogo terminou.
    bool advance();

    bool isActive() const;
    bool isLastLine() const;
    bool isLineComplete() const;

    const std::string& speaker() const;
    std::string visibleText() const;

private:
    std::map<std::string, std::vector<DialogueLine>> m_dialogues;

    const std::vector<DialogueLine>* m_current;
    size_t m_line;
    float m_visibleChars;
    size_t m_lineLength; // em caracteres (nao em bytes)

    void beginLine();

    static size_t utf8Length(const std::string& text);
    static std::string utf8Prefix(const std::string& text, size_t characters);
};

#endif
