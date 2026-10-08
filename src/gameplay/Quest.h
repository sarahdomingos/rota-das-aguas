#ifndef QUEST_H
#define QUEST_H

// Missoes (tarefas) orientadas a dados: assets/missoes/<capitulo>.txt.
// Cada missao tem um objetivo, uma condicao de conclusao (ter N itens ou
// entregar N itens a alguem), dialogos de inicio/andamento/conclusao,
// um desafio opcional (A8) e a proxima missao. A missao fica ligada a um
// objeto/NPC pelo id de dialogo dele (ex.: "vendedora").
// O formato esta documentado no topo de assets/missoes/maceio.txt.

#include "Inventory.h"

#include <map>
#include <string>
#include <vector>

enum class QuestStatus {
    Locked,     // ainda nao liberada (vem depois de outra)
    Available,  // pode ser iniciada falando com quem pede
    Active,     // em andamento
    Done
};

struct QuestDefinition {
    std::string id;
    std::string title;
    std::string objective;
    std::string giver;          // id de dialogo do NPC/objeto que pede e recebe
    bool deliver;               // true: "entregar" (retira os itens); false: "ter"
    std::string itemId;
    int amount;
    bool initial;
    std::string dialogueStart;
    std::string dialogueProgress;
    std::string dialogueComplete;
    std::string challenge;      // id do desafio de matematica (opcional)
    std::string next;           // id da proxima missao (opcional)
};

// O que aconteceu ao fechar um dialogo de missao
struct QuestEvent {
    std::string startedId;
    std::string completedId;
    std::string challengeId;
    int delivered;
};

class QuestLog {
public:
    bool loadFile(const std::string& path);

    // Ao interagir com algo de id "giverId": escolhe o dialogo certo conforme
    // a missao dele (ou devolve o proprio id, se nao houver missao).
    std::string dialogueFor(const std::string& giverId, const Inventory& inventory);

    // Ao fechar o dialogo: inicia ou conclui a missao correspondente.
    QuestEvent onDialogueClosed(Inventory& inventory);

    const QuestDefinition* find(const std::string& id) const;
    const QuestDefinition* activeQuest() const;
    QuestStatus status(const std::string& id) const;
    int progress(const QuestDefinition& quest, const Inventory& inventory) const;

private:
    enum class Pending { None, Start, Complete };

    std::vector<QuestDefinition> m_quests;
    std::map<std::string, QuestStatus> m_status;
    Pending m_pending = Pending::None;
    std::string m_pendingQuest;
};

#endif
