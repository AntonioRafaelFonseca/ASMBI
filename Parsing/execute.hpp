#pragma once
#include <thread>
#include <chrono>
#include <raylib.h>
#include <stdlib.h>
#include <bits/stdc++.h>

#include "tokenize.hpp"
#include "../src/chip.h"

#define DISPLAY_SCALE 4

inline unsigned long long pc = 0;

inline Color GetColorFrom16Bit(halfword h)
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

inline word GetTokenValue(const token& t, chip& c)
{
    if (t.type == INT_LITERAL)
        return t.value;
    if (t.type == REGISTER)
        return c.readReg(static_cast<REGISTER_INDEX>(t.value));
    return 0;
}
inline const std::vector<int> keys = {
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    65,
    66,
    67,
    68,
    69,
    70,
    32,
    264,
    265,
    263,
    262
};

inline int GetKeyDown()
{
    for(size_t i = 0; i < keys.size(); i++)
    {
        if(IsKeyDown(keys.at(i))) return keys.at(i);
    }
    return 0;
}

const size_t startAddr = 0xBFFE;
const size_t endAddr = 0xFFFE;

Image imgBuff{};
Texture txtureBuff{};

inline int updateDisp(bool debugMode)
{

    BeginDrawing();
    DrawTextureEx(txtureBuff, { 0, 0 }, 0.f, DISPLAY_SCALE, WHITE);
    if(debugMode) SetWindowTitle(
        (("ASMBI Display - FPS: " +
            std::to_string(GetFPS())).c_str()
        ));
    EndDrawing();
    return WindowShouldClose();
}
inline int drawDisplay(bool display, chip& destChip, bool debugMode)
{
        if (WindowShouldClose())
        {
            std::cout << "\nClosing Window\n";
            return 1;
        }

        if (destChip.regs.at(rc) != 0x00)
        {
            halfword hlw = destChip.mem.at(0xBFFD);
            Color cl = GetColorFrom16Bit(hlw);
            ImageDrawRectangle(&imgBuff, 0, 0, imgBuff.width, imgBuff.height, cl);
        }

        for (size_t a = startAddr; a < endAddr; a++)
        {
            halfword hw = destChip.mem.at(a);
            if(hw==0) continue;
            Color c = GetColorFrom16Bit(hw);

            size_t pixelIndex = a - startAddr;
            int x = pixelIndex % 128;
            int y = pixelIndex / 128;

            if (x < 128 && y < 64)
                ImageDrawPixel(&imgBuff, x, y, c);
        }

        UpdateTexture(txtureBuff, imgBuff.data);

        BeginDrawing();
        DrawTextureEx(txtureBuff, { 0, 0 }, 0.f, DISPLAY_SCALE, WHITE);
        if(debugMode) SetWindowTitle(
            (("ASMBI Display - FPS: " +
                std::to_string(GetFPS())).c_str()
            ));
        EndDrawing();
    return 0;
}

inline bool execute(const char* filepath, chip& destChip, bool display, bool debugMode)
{
    std::vector<token> tokens = tokenize(filepath, debugMode);


    if (display)
    {
        imgBuff.width = 128;
        imgBuff.height = 64;
        imgBuff.mipmaps = 1;
        imgBuff.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
        imgBuff.data = RL_CALLOC(128 * 64, sizeof(halfword));
        if (imgBuff.data == nullptr)
        {
            std::cerr << "\n[ERROR] Could not allocate the display framebuffer\n";
            return false;
        }

        txtureBuff = LoadTextureFromImage(imgBuff);
        if (txtureBuff.id == 0)
        {
            std::cerr << "\n[ERROR] Could not create the display texture\n";
            UnloadImage(imgBuff);
            return false;
        }
    }

    pc = findLabel("main", 0);
    bool executionSucceeded = true;
    
    while (pc < tokens.size())
    {
        destChip.mem.at(0)++;
        destChip.mem.addrs[1] = GetKeyDown();
        
        token current = tokens.at(pc);
        if(display)
        {
            if (current.type == DISPLAY_DRAW)
            {
                if (drawDisplay(display, destChip, debugMode) == 1) break;
            }
            else if (current.type == DISPLAY_UPDATE)
            {
                if (updateDisp(debugMode) == 1) break;
            }
        }
        if (current.type == SLEEP)
        {
            if (pc + 1 < tokens.size())
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(
                        GetTokenValue(tokens.at(pc + 1), destChip)
                    )
                );

                pc += 2;
            }
            else
            {
                pc++;
            }
        }
        else if (current.type == GOTO)
        {
            pc = current.value;
        }
        else if (
            current.type == GOTO_EQ ||
            current.type == GOTO_NOT_EQ ||
            current.type == GOTO_GT ||
            current.type == GOTO_GT_EQ ||
            current.type == GOTO_LT ||
            current.type == GOTO_LT_EQ)
        {

            if (pc + 3 >= tokens.size())
                break;

            bool condition = false;

            word val1 = GetTokenValue(tokens.at(pc + 1), destChip);
            word val2 = GetTokenValue(tokens.at(pc + 3), destChip);

            if (current.type == GOTO_EQ)
                condition = (val1 == val2);

            else if (current.type == GOTO_NOT_EQ)
                condition = (val1 != val2);

            else if (current.type == GOTO_GT)
                condition = (val1 > val2);

            else if (current.type == GOTO_GT_EQ)
                condition = (val1 >= val2);

            else if (current.type == GOTO_LT)
                condition = (val1 < val2);

            else if (current.type == GOTO_LT_EQ)
                condition = (val1 <= val2);

            if (condition)
                pc = current.value;
            else
                pc += 4;
        }
        else if (current.type == SYSCALL)
        {
            const SYSCALL_RESULT result = destChip.syscall();
            if (result == SYSCALL_RESULT::ERROR)
            {
                executionSucceeded = false;
                break;
            }
            if (result == SYSCALL_RESULT::HALT)
                break;
            pc++;
        }
        else if (current.type == WRITE_REG ||
                 current.type == ADD ||
                 current.type == SUB ||
                 current.type == MULT ||
                 current.type == DIV)
        {
            if (pc + 3 >= tokens.size())
                break;

            REGISTER_INDEX regIndex =
                static_cast<REGISTER_INDEX>(tokens.at(pc + 1).value);

            word val =
                GetTokenValue(tokens.at(pc + 3), destChip);

            if (current.type == WRITE_REG)
                destChip.writeReg(regIndex, val);
            else if (current.type == ADD)
                destChip.regs.at(regIndex) += val;
            else if (current.type == SUB)
                destChip.regs.at(regIndex) -= val;
            else if (current.type == MULT)
                destChip.regs.at(regIndex) *= val;
            else if (current.type == DIV)
            {
                if (val != 0)
                    destChip.regs.at(regIndex) /= val;
                else
                {
                    std::cerr << "\n[ERROR] Division by zero\n";
                    executionSucceeded = false;
                    break;
                }
            }

            pc += 4;
        }
        else
        {
            pc++;
        }
    }

    if (display)
    {
        UnloadTexture(txtureBuff);
        UnloadImage(imgBuff);
    }
    return executionSucceeded;
}
