#pragma once
#include <thread>
#include <chrono>
#include <raylib.h>
#include <stdlib.h>

#include "tokenize.hpp"
#include "../src/chip.h"

#define DISPLAY_SCALE 3

unsigned long long pc = 0;

Color GetColorFrom16Bit(halfword h)
{
  int r5 = (h >> 11) & 0x1F; // 5 bits for Red
  int g6 = (h >> 5)  & 0x3F; // 6 bits for Green
  int b5 = h         & 0x1F; // 5 bits for Blue

  // Scale from 5/6-bit range up to 8-bit (0-255) range accurately
  byte r = static_cast<byte>((r5 * 255) / 31);
  byte g = static_cast<byte>((g6 * 255) / 63);
  byte b = static_cast<byte>((b5 * 255) / 31);

  return {r, g, b, 255};
}

int GetTokenValue(token t, chip& c)
{
    int val = 0;
    if (t.type == INT_LITERAL) {
        val = t.value;
    } else if (t.type == REGISTER) {
        val = c.readReg(static_cast<REGISTER_INDEX>(t.value));
    }
    return val;
}

void execute(const char* filepath, chip& destChip,bool display,bool debugMode)
{
  std::vector<token> tokens = tokenize(filepath, debugMode);

  word destValue = 0;
  word destReg = 0;

  size_t startAddr = 0xBFFE;
  size_t endAddr = 0xFFFE;
  size_t bytesToCopy = endAddr - startAddr;

  Image imgBuff;
  Texture txtureBuff;

  if(display)
  {
      startAddr = 0xBFFE;
      endAddr = 0xFFFE;
      bytesToCopy = endAddr - startAddr;
    
      imgBuff;
      imgBuff.width = 128;
      imgBuff.height = 64;
      imgBuff.mipmaps = 1;
      imgBuff.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
    
      imgBuff.data = RL_MALLOC(bytesToCopy);
    
      txtureBuff = LoadTextureFromImage(imgBuff);
  }
  
  for(pc=findLabel("main", 0);pc<tokens.size();pc+=0)
  {
        pc++;
        destChip.mem.at(0)++;
        destChip.mem.at(1) = GetKeyPressed();

        if(display && !WindowShouldClose())
        {
            for(int a = startAddr; a < endAddr; a++)
            {
                halfword hw = destChip.mem.at(a);
                Color c = GetColorFrom16Bit(hw);

                if(destChip.regs.at(rc) != 0x00)
                {
                    halfword hlw = destChip.mem.at(0xBFFD);
                    Color cl = GetColorFrom16Bit(hlw);
                    if( c.a == 255 ) c = cl;
                }
                size_t pixelIndex = a - startAddr;
                int x = pixelIndex % 128;
                int y = pixelIndex / 128;

                if (x < 128 && y < 64) {
                    ImageDrawPixel(&imgBuff, x, y, c);
                }
            }

            // Atualiza os dados da textura existente em vez de criar uma nova a cada frame
            UpdateTexture(txtureBuff, imgBuff.data);

            BeginDrawing();
            DrawTextureEx(txtureBuff, {0, 0}, 0.f, DISPLAY_SCALE, WHITE);
            EndDrawing();
        }

    token current = tokens.at(pc);
    if(current.type == SLEEP) {std::this_thread::sleep_for(std::chrono::milliseconds(GetTokenValue(tokens.at(pc+1), destChip)));pc+=1;}
    else if(current.type == GOTO) pc = current.value;

    else if(current.type == GOTO_EQ || current.type == GOTO_GT || current.type == GOTO_GT_EQ || current.type == GOTO_LT || current.type == GOTO_LT_EQ)
    {
        bool condition = false;
        int val1 = GetTokenValue(tokens.at(pc+1), destChip);
        int val2 = GetTokenValue(tokens.at(pc+3), destChip);

        if(current.type == GOTO_EQ) condition = (val1 == val2);
        else if(current.type == GOTO_GT) condition = (val1 > val2);
        else if(current.type == GOTO_GT_EQ) condition = (val1 >= val2);
        else if(current.type == GOTO_LT) condition = (val1 < val2);
        else if(current.type == GOTO_LT_EQ) condition = (val1 <= val2);

        if(condition) {
            pc = current.value; // Salta para a label
        } else {
            pc += 3; // Salta os 3 tokens de argumentos (reg, vírgula, valor) para não os executar
        }
    }
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

            val = GetTokenValue(tokens.at(pc+3), destChip);

         destChip.writeReg(regIndex, val);
          // Avança o 'pc' para saltar os tokens já consumidos desta instrução (reg, vírgula, valor)
          pc += 3;
      }
    }
    else if (current.type == ADD)
    {
      // Formato esperado: add [reg], [valor]
      if (pc + 3 < tokens.size())
      {
        REGISTER_INDEX regIndex = static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);
          
        // O valor pode ser um literal inteiro ou o valor de outro registo
        unsigned int val = 0;
        val = GetTokenValue(tokens.at(pc+3), destChip);
        destChip.regs.at(regIndex) += val;
          pc += 3;
      }
    }
    else if (current.type == SUB)
    {
      // Formato esperado: sub [reg], [valor]
      if (pc + 3 < tokens.size())
      {
        REGISTER_INDEX regIndex = static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);
          
        // O valor pode ser um literal inteiro ou o valor de outro registo
        unsigned int val = 0;
        val = GetTokenValue(tokens.at(pc+3), destChip);
        destChip.regs.at(regIndex) -= val;
          pc += 3;
      }
    }
    else if (current.type == MULT)
    {
      // Formato esperado: mult [reg], [valor]
      if (pc + 3 < tokens.size())
      {
        REGISTER_INDEX regIndex = static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);
          
        // O valor pode ser um literal inteiro ou o valor de outro registo
        unsigned int val = 0;
        val = GetTokenValue(tokens.at(pc+3), destChip);
        destChip.regs.at(regIndex) *= val;
          pc += 3;
      }
    }
    else if (current.type == DIV)
    {
      // Formato esperado: div [reg], [valor]
      if (pc + 3 < tokens.size())
      {
        REGISTER_INDEX regIndex = static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);
          
        // O valor pode ser um literal inteiro ou o valor de outro registo
        unsigned int val = 0;
        val = GetTokenValue(tokens.at(pc+3), destChip);
        destChip.regs.at(regIndex) /= val;
          pc += 3;
      }
    }
  }
  if(display)
  {
      UnloadTexture(txtureBuff);
      UnloadImage(imgBuff);
  }
}