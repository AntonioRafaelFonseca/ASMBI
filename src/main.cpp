#include <iostream>
#include <string>
#include <chrono>

#include "raylib.h"
#include "chip.h"

#include "../Parsing/parse.hpp"
#include "../Parsing/tokenize.hpp"
#include "../Parsing/execute.hpp"

int main(int argc, char** argv)
{
  system("cls");
  
  srand(std::chrono::system_clock::now().time_since_epoch().count());

  
  const char* programPath = nullptr;
  bool displayFlag = false;
  bool debugMode = false;

  for (int i = 1; i < argc; ++i)
  {
    const std::string argument = argv[i];
    if (argument == "/D")
      displayFlag = true;
    else if (argument == "/debug")
      
      debugMode = true;
    else if (!argument.empty() &&
             (argument[0] == '/' || argument[0] == '-'))
    {
      std::cerr << "[ERROR] Unknown option: " << argument << '\n'
                << "Usage: " << argv[0]
                << " <program.asmbi> [/D] [/debug]\n";
      return 2;
    }
    else if (programPath == nullptr)
      programPath = argv[i];
    else
    {
      std::cerr << "[ERROR] Only one program file may be specified\n"
                << "Usage: " << argv[0]
                << " <program.asmbi> [/D] [/debug]\n";
      return 2;
    }
  }

  if (programPath == nullptr)
  {
    std::cerr << "Usage: " << argv[0]
              << " <program.asmbi> [/D] [/debug]\n";
    return 2;
  }

  chip c;
  std::cout << "========================================\n"
            << "Compiling & Executing file: " << programPath
            << "\n========================================\n\n";

  SetTraceLogLevel(debugMode ? 0 : 5);
  if(debugMode) std::cout << "\nDEBUG MODE ON\n";
  if (displayFlag)
  {
    InitWindow(128 * DISPLAY_SCALE, 64 * DISPLAY_SCALE, "ASMBI Display");
    SetTargetFPS(60);
    if (!IsWindowReady())
    {
      std::cerr << "[ERROR] Could not initialize the display window\n";
      return 1;
    }
  }

  const bool executionSucceeded =
      execute(programPath, c, displayFlag, debugMode);

  if (displayFlag)
    CloseWindow();
  return executionSucceeded ? 0 : 1;
}