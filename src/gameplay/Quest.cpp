#include "Quest.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

std::string trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& line, char separator)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;
    while (std::getline(stream, part, separator)) {
        parts.push_back(trim(part));
    }
    return parts;
}

}

bool QuestLog::loadFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir as missoes: " << path << std::endl;
        return false;
    }

    std::string line;
    bool hasCurrent = false;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line = line.substr(3);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            QuestDefinition quest;
            quest.id = trim(line.substr(1, line.size() - 2));
            quest.pronoun = "a";
            quest.initial = false;
            m_quests.push_back(quest);
            hasCurrent = true;
            continue;
        }

        size_t colon = line.find(':');
        if (!hasCurrent || colon == std::string::npos) continue;

        QuestDefinition& q = m_quests.back();
        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));

        if (key == "titulo") q.title = value;
        else if (key == "quem") q.giver = value;
        else if (key == "nome") q.giverName = value;
        else if (key == "pronome") q.pronoun = value;
        else if (key == "tarefa") q.task = value;
        else if (key == "item") q.itemKey = value;
        else if (key == "dado") q.facts.push_back(value);
        else if (key == "passo") q.steps.push_back(value);
        else if (key == "inicial") q.initial = (value == "sim");
        else if (key == "dialogo_inicio") q.dialogueStart = value;
        else if (key == "dialogo_erro") q.dialogueWrong = value;
        else if (key == "dialogo_conclusao") q.dialogueComplete = value;
        else if (key == "desafio") q.challenge = value;
        else if (key == "proxima") q.next = value;
        else if (key == "ilustracao") {
            // rotulo | item | quantidade | por fileira
            std::vector<std::string> parts = split(value, '|');
            if (parts.size() >= 3) {
                QuestIllustration illustration;
                illustration.label = parts[0];
                illustration.itemId = parts[1];
                illustration.count = std::atoi(parts[2].c_str());
                illustration.perRow = parts.size() >= 4 ? std::atoi(parts[3].c_str()) : 5;
                if (illustration.perRow < 1) illustration.perRow = 5;
                q.illustrations.push_back(illustration);
            }
        }
    }

    for (const QuestDefinition& quest : m_quests) {
        m_status[quest.id] = quest.initial ? QuestStatus::Available : QuestStatus::Locked;
    }

    return true;
}

const QuestDefinition* QuestLog::openQuestFor(const std::string& giverId) const
{
    for (const QuestDefinition& quest : m_quests) {
        QuestStatus current = status(quest.id);
        if (quest.giver == giverId && (current == QuestStatus::Available || current == QuestStatus::Active)) {
            return &quest;
        }
    }
    return nullptr;
}

void QuestLog::start(const std::string& id)
{
    if (status(id) == QuestStatus::Available) {
        m_status[id] = QuestStatus::Active;
    }
}

void QuestLog::complete(const std::string& id)
{
    m_status[id] = QuestStatus::Done;

    const QuestDefinition* quest = find(id);
    if (quest != nullptr && !quest->next.empty() && status(quest->next) == QuestStatus::Locked) {
        m_status[quest->next] = QuestStatus::Available;
    }
}

void QuestLog::addAttempt(const std::string& id)
{
    m_attempts[id]++;
}

int QuestLog::attempts(const std::string& id) const
{
    std::map<std::string, int>::const_iterator found = m_attempts.find(id);
    return found == m_attempts.end() ? 0 : found->second;
}

const QuestDefinition* QuestLog::find(const std::string& id) const
{
    for (const QuestDefinition& quest : m_quests) {
        if (quest.id == id) return &quest;
    }
    return nullptr;
}

QuestStatus QuestLog::status(const std::string& id) const
{
    std::map<std::string, QuestStatus>::const_iterator found = m_status.find(id);
    return found == m_status.end() ? QuestStatus::Locked : found->second;
}

const QuestDefinition* QuestLog::activeQuest() const
{
    for (const QuestDefinition& quest : m_quests) {
        if (status(quest.id) == QuestStatus::Active) return &quest;
    }
    return nullptr;
}

const std::vector<QuestDefinition>& QuestLog::all() const
{
    return m_quests;
}

bool QuestLog::allDone() const
{
    for (const QuestDefinition& quest : m_quests) {
        if (status(quest.id) != QuestStatus::Done) return false;
    }
    return !m_quests.empty();
}
