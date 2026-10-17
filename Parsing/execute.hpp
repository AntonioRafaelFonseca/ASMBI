#pragma once

#include "tokenize.hpp"
#include "../src/chip.h"

unsigned long long pc = 0;

void execute(const char* filepath, chip& destChip)
{
  std::vector<token> tokens = tokenize(filepath);

  int destReg=0;
  int destValue=0;
  bool writeFlag = 0;
  for(pc=0;pc<tokens.size();pc++)
  {
    token current = tokens.at(pc);
    if(current.type == GOTO) pc = current.value;
    else if(current.type == SYSCALL) destChip.syscall();

    else if(current.type == REGISTER) {
      if(pc > 0 && tokens.at(pc-1).type == COMMA)
          destValue = destChip.readReg(static_cast<REGISTER_INDEX>(current.value));

      else if(pc > 0 && tokens.at(pc-1).type == WRITE_REG)
          destReg = tokens.at(pc-1).value;
    }

    else if(current.type == INT_LITERAL) destValue = current.value;
    else if (current.type == WRITE_REG)
    {
      // Formato esperado: wr [reg], [valor]
      if (pc + 3 < tokens.size())
      {
          REGISTER_INDEX regIndex = static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);
          
          // O valor pode ser um literal inteiro ou o valor de outro registo
          unsigned int val = 0;
          if (tokens.at(pc + 3).type == INT_LITERAL) {
              val = tokens.at(pc + 3).value;
          } else if (tokens.at(pc + 3).type == REGISTER) {
              val = destChip.readReg(static_cast<REGISTER_INDEX>(tokens.at(pc + 3).value));
          }
         destChip.writeReg(regIndex, val);
          // Avança o 'pc' para saltar os tokens já consumidos desta instrução (reg, vírgula, valor)
          pc += 3;
      }
    }
  }
}