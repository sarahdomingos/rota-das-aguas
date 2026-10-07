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

}

bool QuestLog::loadFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir as missoes: " << path << std::endl;
        return false;
    }

    QuestDefinition* current = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line = line.substr(3);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            QuestDefinition quest;
            quest.id = trim(line.substr(1, line.size() - 2));
            quest.deliver = true;
            quest.amount = 1;
            quest.initial = false;
            m_quests.push_back(quest);
            current = &m_quests.back();
            continue;
        }

        size_t colon = line.find(':');
        if (current == nullptr || colon == std::string::npos) continue;

        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));

        if (key == "titulo") current->title = value;
        else if (key == "objetivo") current->objective = value;
        else if (key == "quem") current->giver = value;
        else if (key == "inicial") current->initial = (value == "sim");
        else if (key == "dialogo_inicio") current->dialogueStart = value;
        else if (key == "dialogo_andamento") current->dialogueProgress = value;
        else if (key == "dialogo_conclusao") current->dialogueComplete = value;
        else if (key == "desafio") current->challenge = value;
        else if (key == "proxima") current->next = value;
        else if (key == "condicao") {
            // "entregar coco 3" ou "ter coco 3"
            std::stringstream words(value);
            std::string verb;
            words >> verb >> current->itemId >> current->amount;
            current->deliver = (verb != "ter");
        }
    }

    for (const QuestDefinition& quest : m_quests) {
        m_status[quest.id] = quest.initial ? QuestStatus::Available : QuestStatus::Locked;
    }

    return true;
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

int QuestLog::progress(const QuestDefinition& quest, const Inventory& inventory) const
{
    int count = inventory.count(quest.itemId);
    return count < quest.amount ? count : quest.amount;
}

std::string QuestLog::dialogueFor(const std::string& giverId, const Inventory& inventory)
{
    m_pending = Pending::None;

    for (const QuestDefinition& quest : m_quests) {
        if (quest.giver != giverId) continue;

        QuestStatus current = status(quest.id);

        if (current == QuestStatus::Available) {
            m_pending = Pending::Start;
            m_pendingQuest = quest.id;
            return quest.dialogueStart.empty() ? giverId : quest.dialogueStart;
        }

        if (current == QuestStatus::Active) {
            if (inventory.count(quest.itemId) >= quest.amount) {
                m_pending = Pending::Complete;
                m_pendingQuest = quest.id;
                return quest.dialogueComplete.empty() ? giverId : quest.dialogueComplete;
            }
            return quest.dialogueProgress.empty() ? giverId : quest.dialogueProgress;
        }
    }

    return giverId;
}

QuestEvent QuestLog::onDialogueClosed(Inventory& inventory)
{
    QuestEvent event = { "", "", "", 0 };
    const QuestDefinition* quest = find(m_pendingQuest);

    if (quest != nullptr && m_pending == Pending::Start) {
        m_status[quest->id] = QuestStatus::Active;
        event.startedId = quest->id;
    }
    else if (quest != nullptr && m_pending == Pending::Complete) {
        if (quest->deliver) {
            inventory.remove(quest->itemId, quest->amount);
            event.delivered = quest->amount;
        }

        m_status[quest->id] = QuestStatus::Done;
        event.completedId = quest->id;
        event.challengeId = quest->challenge;

        if (!quest->next.empty() && status(quest->next) == QuestStatus::Locked) {
            m_status[quest->next] = QuestStatus::Available;
        }
    }

    m_pending = Pending::None;
    m_pendingQuest.clear();
    return event;
}
