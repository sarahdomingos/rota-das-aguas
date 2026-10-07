#ifndef MATH_CHALLENGE_H
#define MATH_CHALLENGE_H

// Motor generico dos desafios matematicos, orientado a dados
// (assets/desafios/<capitulo>.txt). Um desafio tem a conta (com variaveis
// preenchidas pelo que o jogador fez, ex.: {entregue}), dificuldade, contexto
// e a acao esperada. A conta e montada e resolvida pelo motor: as 4 operacoes
// e contas combinadas (multiplicacao e divisao antes de soma e subtracao).
// O formato esta documentado no topo de assets/desafios/maceio.txt.

#include <map>
#include <string>
#include <vector>

enum class Operation {
    Add,
    Subtract,
    Multiply,
    Divide,
    Combined
};

struct MathChallenge {
    std::string id;
    std::string context;
    std::string expectedAction;
    int difficulty;

    Operation operation;
    std::vector<int> operands;
    std::vector<char> operators;   // '+', '-', '*', '/'
    int result;
    bool valid;                    // false se a conta nao puder ser resolvida (ex.: divisao nao exata)

    // Conta montada para a tela, ex.: "2 + 3 = 5", "12 ÷ 3 = 4"
    std::string text() const;
};

class MathChallengeBook {
public:
    bool loadFile(const std::string& path);

    // Monta o desafio substituindo as variaveis ({nome}) pelos valores dados.
    bool build(const std::string& id, const std::map<std::string, int>& variables, MathChallenge& out) const;

    // Resolve uma conta (ex.: "2 + 3 * 4"); usado pelo build e util para testes.
    static bool evaluate(const std::string& expression, MathChallenge& out);

private:
    struct Entry {
        std::string expression;
        std::string context;
        std::string action;
        int difficulty;
    };

    std::map<std::string, Entry> m_entries;

    static std::string substitute(const std::string& text, const std::map<std::string, int>& variables);
};

#endif
