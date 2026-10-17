#pragma once

#include "parse.hpp"

enum TOKEN_TYPE
{
  LABEL,
  GOTO,
  LABEL_NAME,
  WRITE_REG,
  SYSCALL,
  INT_LITERAL,
  REGISTER,
  COMMA,
  COLON
};

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
    std::cerr << "invalid reg";
    exit(1);
  }
  return hexToBin(s.at(1));
}

token getToken(std::string s)
{
  if(s == "lbl") return {LABEL, 0};
  else if(s == "goto") return {GOTO, 0};
  else if(s == "wr") return {WRITE_REG, 0};
  else if(s == "syscall") return {SYSCALL, 0};
  else if(s == ",") return {COMMA, 0};
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
            outVec.push_back(line);
        }
    }
    return outVec;
}

std::vector<token> tokenize(const char* filepath)
{
    std::vector<token> outVec;
    
    // Lê e simplifica o código
    std::string str = readsimplifyCode(filepath);
    std::vector<std::string> inVec = getArrayString(str);

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
            t.value = static_cast<unsigned int>(findLabel(inVec.at(i+1)));
            outVec.push_back(t);
            i++;
        }
        else
        {
            // Para todos os outros tokens (GOTO, WRITE_REG, SYSCALL, vírgulas, dois-pontos, nomes de labels)
            outVec.push_back(t);
        }
    }
    
    return outVec;
}
