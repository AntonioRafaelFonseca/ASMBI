#include "iostream"
#include "raylib.h"
#include "chip.h"

#include "../Parsing/parse.hpp"
#include "../Parsing/tokenize.hpp"
#include "../Parsing/execute.hpp"

int main(int argc, char** argv)
{
  chip c;
  system("cls");

  std::cout << "========================================\nCompiling & Executing file: " << argv[1]<<"\n========================================\n\n";
  
  execute(argv[1], c);
  
  return 0;
}