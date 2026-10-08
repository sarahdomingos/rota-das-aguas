#include "Dialogue.h"

#include <fstream>
#include <iostream>

namespace {

const float LETTERS_PER_SECOND = 45.0f;

std::string trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

}

Dialogue::Dialogue()
    : m_current(nullptr),
      m_line(0),
      m_visibleChars(0.0f),
      m_lineLength(0)
{
}

bool Dialogue::loadFile(const std::string& path)
{
    std::ifstream file(path.c_str());

    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir os dialogos: " << path << std::endl;
        return false;
    }

    std::string line;
    std::string currentId;
    bool firstLine = true;

    while (std::getline(file, line)) {
        // Ignora o BOM do UTF-8, se o editor tiver salvo com ele
        if (firstLine && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            line = line.substr(3);
        }
        firstLine = false;

        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            currentId = trim(line.substr(1, line.size() - 2));
            m_dialogues[currentId].clear();
            continue;
        }

        if (currentId.empty()) {
            continue;
        }

        DialogueLine entry;
        size_t colon = line.find(':');

        if (colon != std::string::npos) {
            entry.speaker = trim(line.substr(0, colon));
            entry.text = trim(line.substr(colon + 1));
        }
        else {
            entry.text = line;
        }

        if (entry.speaker == "Narrador") {
            entry.speaker.clear();
        }

        m_dialogues[currentId].push_back(entry);
    }

    return true;
}

bool Dialogue::has(const std::string& id) const
{
    std::map<std::string, std::vector<DialogueLine>>::const_iterator found = m_dialogues.find(id);
    return found != m_dialogues.end() && !found->second.empty();
}

bool Dialogue::start(const std::string& id)
{
    if (!has(id)) {
        std::cerr << "Dialogo nao encontrado: " << id << std::endl;
        m_current = nullptr;
        return false;
    }

    m_current = &m_dialogues[id];
    m_line = 0;
    beginLine();
    return true;
}

void Dialogue::beginLine()
{
    m_visibleChars = 0.0f;
    m_lineLength = utf8Length((*m_current)[m_line].text);
}

void Dialogue::update(float deltaTime)
{
    if (isActive() && !isLineComplete()) {
        m_visibleChars += LETTERS_PER_SECOND * deltaTime;
    }
}

bool Dialogue::advance()
{
    if (!isActive()) {
        return false;
    }

    if (!isLineComplete()) {
        m_visibleChars = static_cast<float>(m_lineLength);
        return true;
    }

    if (m_line + 1 < m_current->size()) {
        ++m_line;
        beginLine();
        return true;
    }

    m_current = nullptr;
    return false;
}

bool Dialogue::isActive() const
{
    return m_current != nullptr;
}

bool Dialogue::isLastLine() const
{
    return isActive() && m_line + 1 >= m_current->size();
}

bool Dialogue::isLineComplete() const
{
    return m_visibleChars >= static_cast<float>(m_lineLength);
}

const std::string& Dialogue::speaker() const
{
    static const std::string empty;
    return isActive() ? (*m_current)[m_line].speaker : empty;
}

std::string Dialogue::visibleText() const
{
    if (!isActive()) {
        return "";
    }
    return utf8Prefix((*m_current)[m_line].text, static_cast<size_t>(m_visibleChars));
}

size_t Dialogue::utf8Length(const std::string& text)
{
    size_t count = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

std::string Dialogue::utf8Prefix(const std::string& text, size_t characters)
{
    size_t count = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if ((c & 0xC0) != 0x80) {
            if (count == characters) {
                return text.substr(0, i);
            }
            ++count;
        }
    }
    return text;
}
