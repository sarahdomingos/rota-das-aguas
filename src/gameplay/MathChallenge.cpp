// Motor generico dos desafios matematicos

#include "MathChallenge.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace {

std::string trim(const std::string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

std::string toString(int value)
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    return buffer;
}

const char* symbol(char op)
{
    switch (op) {
    case '+': return "+";
    case '-': return "-";
    case '*': return "×";
    case '/': return "÷";
    default:  return "?";
    }
}

}

std::string MathChallenge::text() const
{
    if (operands.empty()) {
        return "";
    }

    std::string out = toString(operands[0]);
    for (size_t i = 0; i < operators.size() && i + 1 < operands.size(); ++i) {
        out += std::string(" ") + symbol(operators[i]) + " " + toString(operands[i + 1]);
    }

    out += valid ? " = " + toString(result) : " = ?";
    return out;
}

bool MathChallengeBook::loadFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Nao foi possivel abrir os desafios: " << path << std::endl;
        return false;
    }

    Entry* current = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line = line.substr(3);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            current = &m_entries[trim(line.substr(1, line.size() - 2))];
            current->difficulty = 1;
            continue;
        }

        size_t colon = line.find(':');
        if (current == nullptr || colon == std::string::npos) continue;

        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));

        if (key == "conta") current->expression = value;
        else if (key == "contexto") current->context = value;
        else if (key == "acao") current->action = value;
        else if (key == "dificuldade") current->difficulty = std::atoi(value.c_str());
    }

    return true;
}

std::string MathChallengeBook::substitute(const std::string& text, const std::map<std::string, int>& variables)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '{') {
            size_t close = text.find('}', i);
            if (close != std::string::npos) {
                std::string name = text.substr(i + 1, close - i - 1);
                std::map<std::string, int>::const_iterator found = variables.find(name);
                out += found != variables.end() ? toString(found->second) : "0";
                i = close;
                continue;
            }
        }
        out += text[i];
    }
    return out;
}

bool MathChallengeBook::evaluate(const std::string& expression, MathChallenge& out)
{
    out.operands.clear();
    out.operators.clear();
    out.valid = false;
    out.result = 0;

    // Separa numeros e operadores (aceita tambem x para multiplicar e : para dividir)
    std::string number;
    for (size_t i = 0; i <= expression.size(); ++i) {
        char c = i < expression.size() ? expression[i] : ' ';

        if (std::isdigit(static_cast<unsigned char>(c))) {
            number += c;
            continue;
        }

        if (!number.empty()) {
            out.operands.push_back(std::atoi(number.c_str()));
            number.clear();
        }

        if (c == '+' || c == '-' || c == '*' || c == '/') out.operators.push_back(c);
        else if (c == 'x' || c == 'X') out.operators.push_back('*');
        else if (c == ':') out.operators.push_back('/');
    }

    if (out.operands.empty() || out.operators.size() + 1 != out.operands.size()) {
        return false;
    }

    // Tipo da operacao: uma so ou combinada
    out.operation = Operation::Combined;
    if (out.operators.size() == 1) {
        switch (out.operators[0]) {
        case '+': out.operation = Operation::Add; break;
        case '-': out.operation = Operation::Subtract; break;
        case '*': out.operation = Operation::Multiply; break;
        case '/': out.operation = Operation::Divide; break;
        }
    }

    // 1o passo: multiplicacao e divisao (da esquerda para a direita)
    std::vector<int> terms;
    std::vector<char> additive;
    int current = out.operands[0];

    for (size_t i = 0; i < out.operators.size(); ++i) {
        char op = out.operators[i];
        int next = out.operands[i + 1];

        if (op == '*') {
            current *= next;
        }
        else if (op == '/') {
            if (next == 0 || current % next != 0) {
                return false; // so contas com resultado inteiro
            }
            current /= next;
        }
        else {
            terms.push_back(current);
            additive.push_back(op);
            current = next;
        }
    }
    terms.push_back(current);

    // 2o passo: soma e subtracao
    int result = terms[0];
    for (size_t i = 0; i < additive.size(); ++i) {
        result = additive[i] == '+' ? result + terms[i + 1] : result - terms[i + 1];
    }

    out.result = result;
    out.valid = true;
    return true;
}

bool MathChallengeBook::build(const std::string& id, const std::map<std::string, int>& variables, MathChallenge& out) const
{
    std::map<std::string, Entry>::const_iterator found = m_entries.find(id);
    if (found == m_entries.end()) {
        std::cerr << "Desafio nao encontrado: " << id << std::endl;
        return false;
    }

    out.id = id;
    out.context = substitute(found->second.context, variables);
    out.expectedAction = substitute(found->second.action, variables);
    out.difficulty = found->second.difficulty;

    evaluate(substitute(found->second.expression, variables), out);
    return true;
}
