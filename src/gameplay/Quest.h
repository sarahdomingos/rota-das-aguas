#ifndef QUEST_H
#define QUEST_H

// Missoes orientadas a dados: assets/missoes/<capitulo>.txt.
// Cada missao tem quem pede (NPC), a tarefa, os dados do problema e o passo a
// passo do quadro da missao, a ilustracao dos itens e um desafio de
// matematica (assets/desafios) cujo resultado e a resposta: na entrega, a Lia
// escolhe o resultado da conta e precisa ter exatamente essa quantidade do
// item na mochila. As tentativas de entrega sao contadas por missao.
// O formato esta documentado no topo de assets/missoes/maceio.txt.

#include <map>
#include <string>
#include <vector>

enum class QuestStatus {
    Locked,     // ainda nao liberada (vem depois de outra)
    Available,  // pode ser iniciada falando com quem pede
    Active,     // em andamento
    Done
};

// Grupo de itens desenhado no quadro da missao (ex.: "A Dona Graça tem" 20 cocos, 5 por fileira)
struct QuestIllustration {
    std::string label;
    std::string itemId;
    int count;
    int perRow;
};

struct QuestDefinition {
    std::string id;
    std::string title;
    std::string giver;          // id de dialogo do NPC que pede e recebe (ex.: "vendedora")
    std::string giverName;      // nome para a tela (ex.: "Dona Graça")
    std::string pronoun;        // "a" ou "o" (Ajude-a / Ajude-o)
    std::string task;           // o que a Lia deve fazer
    std::string itemKey;        // id do item ou categoria que sera entregue
    std::vector<std::string> facts;
    std::vector<std::string> steps;
    std::vector<QuestIllustration> illustrations;
    bool initial;
    std::string dialogueStart;
    std::string dialogueWrong;
    std::string dialogueComplete;
    std::string challenge;      // id do desafio: o resultado da conta e a quantidade a entregar
    std::string next;           // id da proxima missao (opcional)
};

class QuestLog {
public:
    bool loadFile(const std::string& path);

    // Missao ligada a quem a Lia esta falando que ainda nao terminou (ou nullptr)
    const QuestDefinition* openQuestFor(const std::string& giverId) const;

    void start(const std::string& id);
    void complete(const std::string& id);   // conclui e libera a proxima

    void addAttempt(const std::string& id);
    int attempts(const std::string& id) const;

    const QuestDefinition* find(const std::string& id) const;
    const QuestDefinition* activeQuest() const;
    QuestStatus status(const std::string& id) const;

    const std::vector<QuestDefinition>& all() const;
    bool allDone() const;

private:
    std::vector<QuestDefinition> m_quests;
    std::map<std::string, QuestStatus> m_status;
    std::map<std::string, int> m_attempts;
};

#endif
