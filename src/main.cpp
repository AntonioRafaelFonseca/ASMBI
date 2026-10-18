#include <iostream>
#include <cstring>
#include "raylib.h"
#include "chip.h"

#include "../Parsing/parse.hpp"
#include "../Parsing/tokenize.hpp"
#include "../Parsing/execute.hpp"

int main(int argc, char** argv)
{

  SetTraceLogLevel(5);

  chip c;
  system("cls");
    
    std::cout << "========================================\nCompiling & Executing file: " << argv[1] << "\n========================================\n\n";
    
    // Verifica corretamente a flag /D usando strcmp e prevenindo segmentation fault
    unsigned char displayFlag = ((argc > 2 && strcmp(argv[2], "/D") == 0) || (argc > 3 && strcmp(argv[3], "/D") == 0)) ? 0x01 : 0x00;
    unsigned char debugMode = ((argc > 3 && strcmp(argv[3], "/debug") == 0) || (argc > 2 && strcmp(argv[2], "/debug") == 0)) ? 0x01 : 0x00;
    
    if(displayFlag) InitWindow(128 * DISPLAY_SCALE, 64 * DISPLAY_SCALE, "ASMBI Display");

    execute(argv[1], c, displayFlag, debugMode);
  
    CloseWindow();
    return 0;
}