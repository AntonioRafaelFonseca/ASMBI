#pragma once

#include "parse.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

enum TOKEN_TYPE
{
    LABEL,

    GOTO,
    GOTO_EQ,
    GOTO_NOT_EQ,
    GOTO_LT,
    GOTO_GT,
    GOTO_LT_EQ,
    GOTO_GT_EQ,

    LABEL_NAME,

    WRITE_REG,
    SYSCALL,

    ADD,
    SUB,
    MULT,
    DIV,

    MOD,

    INT_LITERAL,
    REGISTER,

    COMMA,
    COLON,

    DISPLAY_DRAW,
    DISPLAY_UPDATE,


    SLEEP
};

inline std::string GetTokenTypeSTR(TOKEN_TYPE t)
{
    switch (t)
    {
    case LABEL:        return "LABEL";
    case LABEL_NAME:   return "LABEL NAME";

    case GOTO:         return "GOTO";
    case GOTO_EQ:      return "GOTO IF EQUAL";
    case GOTO_NOT_EQ:  return "GOTO IF NOT EQUAL";
    case GOTO_LT:      return "GOTO IF LESS THAN";
    case GOTO_GT:      return "GOTO IF GREATER THAN";
    case GOTO_LT_EQ:   return "GOTO IF LESS OR EQUAL TO";
    case GOTO_GT_EQ:   return "GOTO IF GREATER OR EQUAL TO";

    case WRITE_REG:    return "WRITE REGISTER";
    case SYSCALL:      return "SYSCALL";

    case ADD:          return "ADD";
    case SUB:          return "SUBTRACT";
    case MULT:         return "MULTIPLY";
    case DIV:          return "DIVIDE";

    case MOD:          return "MOD";

    case INT_LITERAL:  return "INTEGER LITERAL";
    case REGISTER:     return "REGISTER";

    case COMMA:        return "COMMA";
    case COLON:        return "COLON";
    case SLEEP:        return "SLEEP";
    case DISPLAY_DRAW:       return "DISPLAY DRAW";
    case DISPLAY_UPDATE:       return "DISPLAY UPDATE";


    default:           return "UNKNOWN TOKEN";
    }
}

struct token
{
    TOKEN_TYPE type;
    unsigned int value;
};

inline int hexToBin(char ascii)
{
    if (ascii >= '0' && ascii <= '9') return ascii - '0';
    if (ascii >= 'a' && ascii <= 'f') return ascii - 'a' + 10;
    if (ascii >= 'A' && ascii <= 'F') return ascii - 'A' + 10;
    return -1;
}

inline unsigned int getReg(const std::string& s)
{
    if (s == "rr") return 16;
    if (s.size() != 2 || s[0] != 'r' || hexToBin(s[1]) < 0)
    {
        std::cerr << "\n[ERROR] Invalid register '" << s
                  << "'; expected r0 through rf\n";
        exit(1);
    }
    return static_cast<unsigned int>(hexToBin(s[1]));
}

inline unsigned int parseInteger(const std::string& text, int base)
{
    try
    {
        if (text.empty())
            throw std::invalid_argument("empty integer literal");
        for (char digit : text)
        {
            const int value = hexToBin(digit);
            if (value < 0 || value >= base)
                throw std::invalid_argument("invalid digit");
        }

        size_t parsed = 0;
        const unsigned long long value = std::stoull(text, &parsed, base);
        if (parsed != text.size() ||
            value > std::numeric_limits<unsigned int>::max())
            throw std::out_of_range("integer literal");
        return static_cast<unsigned int>(value);
    }
    catch (const std::exception&)
    {
        std::cerr << "\n[ERROR] Invalid or out-of-range integer literal: '"
                  << text << "'\n";
        exit(1);
    }
}

inline token getToken(const std::string& s)
{
    if (s.empty())
    {
        std::cerr << "\n[ERROR] Empty token\n";
        exit(1);
    }

    if(s == "lbl") return {LABEL, 0};

    else if(s == "goto")    return {GOTO, 0};
    else if(s == "geq")     return {GOTO_EQ, 0};
    else if(s == "gneq")    return {GOTO_NOT_EQ, 0};
    else if(s == "glt")     return {GOTO_LT, 0};
    else if(s == "glte")    return {GOTO_LT_EQ, 0};
    else if(s == "ggt")     return {GOTO_GT, 0};
    else if(s == "ggte")    return {GOTO_GT_EQ, 0};

    else if(s == "wr")      return {WRITE_REG, 0};
    else if(s == "syscall") return {SYSCALL, 0};

    else if(s == "add")     return {ADD, 0};
    else if(s == "sub")     return {SUB, 0};
    else if(s == "mult")    return {MULT, 0};
    else if(s == "div")     return {DIV, 0};

    else if(s == "mod")     return {MOD, 0};

    else if(s == "slp")     return {SLEEP, 0};
    else if(s == "DRAW")     return {DISPLAY_DRAW, 0};
    else if(s == "UPDATE")     return {DISPLAY_UPDATE, 0};
    else if(s == ":")       return {COLON, 0};
    else if(s == ",")       return {COMMA, 0};
    else if(s[0] == 'r' && s.length() == 2) return {REGISTER, getReg(s)};
    else if(
        s.find_first_not_of
        (
        "0123456789"
        ) ==
        std::string::npos)    return {INT_LITERAL, 0};
    else return {LABEL_NAME, 0};
}

inline std::vector<std::string> getArrayString(const std::string& s)
{
    std::vector<std::string> outVec;
    std::stringstream ss(s);
    std::string line;

    while (std::getline(ss, line))
    {
        if (!line.empty()) {
            std::string lineA = line.c_str();

            if (line.size() >= 2 &&
                line[0] == '0' &&
                (line[1] == 'x'))
            {
                lineA = std::to_string(parseInteger(line.substr(2), 16));
            }
            else if (line.size() >= 2 &&
                    line[0] == '0' &&
                    (line[1] == 'b'))
            {
                lineA = std::to_string(parseInteger(line.substr(2), 2));
            }

            outVec.push_back(lineA);
        }
    }
    return outVec;
}

inline std::vector<token> tokenize(const char* filepath, bool debugMode)
{
    std::vector<token> outVec;
    std::vector<std::string> sourceVec;
    const auto appendToken = [&outVec, &sourceVec](const token& value, const std::string& source)
    {
        outVec.push_back(value);
        sourceVec.push_back(source);
    };

    std::string str = readsimplifyCode(filepath);
    std::vector<std::string> inVec = getArrayString(str);

    GlobalLabels.clear();
    {
        size_t simulated_pc = 0;
        for (size_t i = 0; i < inVec.size(); ++i)
        {
            token t = getToken(inVec[i]);
            if (t.type == LABEL)
            {
                if (i + 2 >= inVec.size() || getToken(inVec[i + 2]).type != COLON)
                {
                    std::cerr << "\n[ERROR] Invalid label declaration near token " << i << '\n';
                    exit(1);
                }
                if (getToken(inVec[i + 1]).type != LABEL_NAME)
                {
                    std::cerr << "\n[ERROR] Label names must be identifiers near token "
                              << i << '\n';
                    exit(1);
                }
                const std::string& labelName = inVec[i + 1];
                for (const auto& label : GlobalLabels)
                {
                    if (label.name == labelName)
                    {
                        std::cerr << "\n[ERROR] Duplicate label: " << labelName << '\n';
                        exit(1);
                    }
                }
                GlobalLabels.push_back({labelName, static_cast<int>(simulated_pc)});
                i += 2;
            }
            else if (t.type == GOTO)
            {
                simulated_pc += 1;
                i++;
            }
            else if (
                t.type == GOTO_EQ ||
                t.type == GOTO_NOT_EQ ||
                t.type == GOTO_LT ||
                t.type == GOTO_GT ||
                t.type == GOTO_LT_EQ ||
                t.type == GOTO_GT_EQ)
            {
                simulated_pc += 4;
                i += 5;
            }
            else
            {
                simulated_pc += 1;
            }
        }
    }

    for (size_t i = 0; i < inVec.size(); i++)
    {
        std::string& word = inVec.at(i);
        token t = getToken(word);

        if (t.type == LABEL)
        {
            i += 2;
            continue;
        }
        else if (t.type == GOTO)
        {
            if (i + 1 >= inVec.size() ||
                getToken(inVec[i + 1]).type != LABEL_NAME)
            {
                std::cerr << "\n[ERROR] Expected a label after 'goto'\n";
                exit(1);
            }
            t.value = static_cast<unsigned int>(findLabel(inVec.at(i+1), i+1));
            appendToken(t, word + " " + inVec.at(i + 1));
            i++;
        }
        else if (
            t.type == GOTO_EQ ||
            t.type == GOTO_NOT_EQ ||
            t.type == GOTO_LT ||
            t.type == GOTO_GT ||
            t.type == GOTO_LT_EQ ||
            t.type == GOTO_GT_EQ)
        {
            if (i + 5 >= inVec.size() ||
                (getToken(inVec[i + 1]).type != REGISTER &&
                 getToken(inVec[i + 1]).type != INT_LITERAL) ||
                getToken(inVec[i + 2]).type != COMMA ||
                (getToken(inVec[i + 3]).type != REGISTER &&
                 getToken(inVec[i + 3]).type != INT_LITERAL) ||
                getToken(inVec[i + 4]).type != COMMA ||
                getToken(inVec[i + 5]).type != LABEL_NAME)
            {
                std::cerr << "\n[ERROR] Invalid conditional branch near token "
                          << i << '\n';
                exit(1);
            }
            t.value = static_cast<unsigned int>(findLabel(inVec.at(i+5), i+5));
            const std::string instructionSource =
                word + " " + inVec.at(i + 1) + inVec.at(i + 2) +
                " " + inVec.at(i + 3) + inVec.at(i + 4) +
                " " + inVec.at(i + 5);
            appendToken(t, instructionSource);

            token op1 = getToken(inVec.at(i+1));
            if (op1.type == INT_LITERAL)
                op1.value = parseInteger(inVec.at(i+1), 10);
            appendToken(op1, inVec.at(i + 1));

            appendToken(getToken(inVec.at(i + 2)), inVec.at(i + 2));

            token op2 = getToken(inVec.at(i+3));
            if (op2.type == INT_LITERAL)
                op2.value = parseInteger(inVec.at(i+3), 10);
            appendToken(op2, inVec.at(i + 3));

            i += 5;
        }
        else if (t.type == WRITE_REG || t.type == ADD ||
                 t.type == SUB || t.type == MULT || t.type == DIV || t.type == MOD)
        {
            if (i + 3 >= inVec.size() ||
                getToken(inVec[i + 1]).type != REGISTER ||
                getToken(inVec[i + 2]).type != COMMA ||
                (getToken(inVec[i + 3]).type != REGISTER &&
                 getToken(inVec[i + 3]).type != INT_LITERAL))
            {
                std::cerr << "\n[ERROR] Invalid "
                          << GetTokenTypeSTR(t.type)
                          << " instruction near token " << i << '\n';
                exit(1);
            }
            appendToken(t, word);
            appendToken(getToken(inVec[i + 1]), inVec[i + 1]);
            appendToken(getToken(inVec[i + 2]), inVec[i + 2]);

            token operand = getToken(inVec[i + 3]);
            if (operand.type == INT_LITERAL)
                operand.value = parseInteger(inVec[i + 3], 10);
            appendToken(operand, inVec[i + 3]);
            i += 3;
        }
        else if (t.type == SLEEP)
        {
            if (i + 1 >= inVec.size() ||
                (getToken(inVec[i + 1]).type != REGISTER &&
                 getToken(inVec[i + 1]).type != INT_LITERAL))
            {
                std::cerr << "\n[ERROR] Expected a duration after 'slp'\n";
                exit(1);
            }
            appendToken(t, word);
            token duration = getToken(inVec[i + 1]);
            if (duration.type == INT_LITERAL)
                duration.value = parseInteger(inVec[i + 1], 10);
            appendToken(duration, inVec[i + 1]);
            i++;
        }
        else if (t.type == INT_LITERAL || t.type == REGISTER ||
                 t.type == LABEL_NAME || t.type == COMMA || t.type == COLON)
        {
            std::cerr << "\n[ERROR] Unexpected token '" << word
                      << "' near source token " << i << '\n';
            exit(1);
        }
        else
        {
            appendToken(t, word);
        }
    }

    if(debugMode)
    {
        std::cout << "\n==================\n";
        std::cout << "   Labels Found   \n";
        std::cout << "==================\n";
        for(size_t i = 0; i < GlobalLabels.size(); i++)
        {
            std::cout << "\nLabel: " << GlobalLabels.at(i).name << " at PC-Index: " << GlobalLabels.at(i).pcIndex <<"\n";
        }
    
        constexpr int indexWidth = 10;
        constexpr int typeWidth = 30;
        constexpr int valueWidth = 12;
        const std::string separator(indexWidth + typeWidth + valueWidth + 32, '-');

        std::cout << "\nASMBI DEBUG - OUTPUT TOKENS\n"
                  << separator << '\n'
                  << std::left << std::setw(indexWidth) << "OUT INDEX"
                  << std::setw(typeWidth) << "TOKEN TYPE"
                  << std::right << std::setw(valueWidth) << "VALUE"
                  << "   " << std::left << "SOURCE\n"
                  << separator << '\n';
        for(size_t i = 0; i < outVec.size(); i++)
        {
            std::cout << std::left << std::setw(indexWidth) << i
                      << std::setw(typeWidth) << GetTokenTypeSTR(outVec.at(i).type)
                      << std::right << std::setw(valueWidth) << outVec.at(i).value
                      << "   " << std::left << sourceVec.at(i) << '\n';
        }
        std::cout << separator << "\n\n" << std::flush;
    }
    return outVec;
}