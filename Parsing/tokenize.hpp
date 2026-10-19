#pragma once

#include "parse.hpp"

#include <iomanip>


enum TOKEN_TYPE
{
  LABEL,

  GOTO,
  GOTO_EQ,
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
  
  INT_LITERAL,
  REGISTER,
  
  COMMA,
  COLON,

  SLEEP
};
std::string GetTokenTypeSTR(TOKEN_TYPE t)
{
    switch (t)
    {
    case LABEL:        return "LABEL";
    case LABEL_NAME:   return "LABEL NAME";

    case GOTO:         return "GOTO";
    case GOTO_EQ:      return "GOTO IF EQUAL";
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

    case INT_LITERAL:  return "INTEGER LITERAL";
    case REGISTER:     return "REGISTER";

    case COMMA:        return "COMMA";
    case COLON:        return "COLON";
    case SLEEP:        return "SLEEP";

    default:           return "UNKNOWN TOKEN";
    }
}


struct token
{
  TOKEN_TYPE type;
  unsigned int value;
};

char hexToBin(char ascii)
{
  if (ascii > 0x40) ascii += 9;
  return (ascii & 0x0F);
}

unsigned int getReg(std::string s)
{
  if(s.at(0) != 'r')
  {
    std::cerr << "[error] Invalid register" << s;
    exit(1);
  }
  return hexToBin(s.at(1));
}

token getToken(std::string s)
{
  if(s == "lbl") return {LABEL, 0};

  else if(s == "goto") return {GOTO, 0};
  else if(s == "geq") return {GOTO_EQ, 0};
  else if(s == "glt") return {GOTO_LT, 0};
  else if(s == "glte") return {GOTO_LT_EQ, 0};
  else if(s == "ggt") return {GOTO_GT, 0};
  else if(s == "ggte") return {GOTO_GT_EQ, 0};
  
  else if(s == "wr") return {WRITE_REG, 0};
  else if(s == "syscall") return {SYSCALL, 0};
  
  else if(s == "add") return {ADD, 0};
  else if(s == "sub") return {SUB, 0};
  else if(s == "mult") return {MULT, 0};
  else if(s == "sleep") return {MULT, 0};
  else if(s == "div") return {DIV, 0};

  else if(s == "slp") return {SLEEP, 0};
  else if(s == ":") return {COLON, 0};
  else if(s.at(0) == 'r') return {REGISTER, getReg(s)};
  else if(s.find_first_not_of("0123456789") == std::string::npos) return {INT_LITERAL, 0};
  else return {LABEL_NAME, 0};
}

std::vector<std::string> getArrayString(std::string s)
{
    std::vector<std::string> outVec;
    std::stringstream ss(s);
    std::string line;
    
    // Lê cada linha separada por '\n' gerada pelo readsimplifyCode
    while (std::getline(ss, line))
    {
        if (!line.empty()) {
            std::string lineA = line.c_str();
            if (line[0] == '0' && line[1] == 'x') lineA = std::to_string(std::stoi(line, 0, 16));
            else if (line[0] == '0' && line[1] == 'b') lineA = std::to_string(std::stoi(replaceStr(line, "0b", ""), 0, 2));
            outVec.push_back(lineA);
        }
    }
    return outVec;
}

std::vector<token> tokenize(const char* filepath, bool debugMode)
{
    std::vector<token> outVec;
    // Lê e simplifica o código
    std::string str = readsimplifyCode(filepath);
    std::vector<std::string> inVec = getArrayString(str);
    if(debugMode)
    {
        system("cls");
        constexpr int indexWidth = 8;
        constexpr int typeWidth = 34;
        const std::string separator(indexWidth + typeWidth + 24, '-');

        std::cout << "\nASMBI DEBUG - SOURCE TOKENS\n"
                  << separator << '\n'
                  << std::left << std::setw(indexWidth) << "INDEX"
                  << std::setw(typeWidth) << "TOKEN TYPE"
                  << "SOURCE\n"
                  << separator << '\n';
        for(size_t i = 0; i < inVec.size(); i++)
        {
            const std::string& source = inVec.at(i);
            std::cout << std::left << std::setw(indexWidth) << i
                      << std::setw(typeWidth) << GetTokenTypeSTR(getToken(source).type)
                      << source << '\n';
        }
        std::cout << separator << "\n\n";
    }

    for (size_t i = 0; i < inVec.size(); i++)
    {
        std::string& word = inVec.at(i);

        token t = getToken(word);

        if (t.type == LABEL)
        {
            // Se encontrar 'lbl', a palavra seguinte (i + 1) deve ser o nome da label
            if (i + 1 < inVec.size())
            {
                std::string labelName = inVec.at(i + 1);
                GlobalLabels.push_back({labelName, static_cast<int>(outVec.size())});
                i++;
            }
            outVec.push_back(t);
        }
        else if (t.type == INT_LITERAL)
        {
            // Converte a string numérica para inteiro e guarda no campo 'value' do token
            t.value = static_cast<unsigned int>(std::stoi(word));
            outVec.push_back(t);
        }
        else if (t.type == GOTO)
        {
            // Converte a string numérica para inteiro e guarda no campo 'value' do token
            t.value = static_cast<unsigned int>(findLabel(inVec.at(i+1), i+1));
            outVec.push_back(t);
            i++;
        }
        else if (t.type == GOTO_EQ || t.type == GOTO_LT || t.type == GOTO_GT || t.type == GOTO_LT_EQ || t.type == GOTO_GT_EQ)
        {
            t.value = static_cast<unsigned int>(findLabel(inVec.at(i+5), i+5));
            outVec.push_back(t);

            // Primeiro operando (ex: r1)
            token op1 = getToken(inVec.at(i+1));
            if (op1.type == INT_LITERAL) op1.value = std::stoi(inVec.at(i+1));
            outVec.push_back(op1);

            // Vírgula
            outVec.push_back(getToken(inVec.at(i+2)));

            // Segundo operando (ex: r5)
            token op2 = getToken(inVec.at(i+3));
            if (op2.type == INT_LITERAL) op2.value = std::stoi(inVec.at(i+3));
            outVec.push_back(op2);

            // Salta os 4 elementos consumidos desta instrução (glte r1, r2, main = 5 palavras, i avança 4)
            i += 5;
        }
        else
        {
            outVec.push_back(t);
        }
    }
    
    return outVec;
}
