#pragma once

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct LBL
{
  std::string name;
  int pcIndex;
};

inline std::vector<LBL> GlobalLabels;

inline int findLabel(std::string s, int tokenLocIndex, bool inVec=0)
{
  for(std::size_t i = 0; i < GlobalLabels.size(); i++)
  {
    auto l = GlobalLabels.at(i);
    if (l.name == s && !inVec) return l.pcIndex;
    if (l.name == s && inVec) return static_cast<int>(i);
  }
  if(s=="main")
  {
    std::cerr << "\n[ERROR] No main label found suggest adding \"lbl main\" at the start of the program";
    exit(1);
  }
  else
  {
    std::cerr << "\n[ERROR] Token: " << std::bitset<16>(tokenLocIndex) << " - " << tokenLocIndex << " Invalid label '" << s << "'";
    exit(1);
  }
  return INT_MAX;
}

inline std::string replaceStr(std::string& c, const std::string a, const std::string b) {
  if (a.empty()) return "";
  size_t pos = c.find(a);
  while (pos != std::string::npos) {
      c.replace(pos, a.length(), b);
      pos = c.find(a, pos + b.length());
  }
  return c;
}

inline std::string readsimplifyCode(const char* filePath)
{
  
  std::ifstream file;
  std::string CfilePath = "";

  fs::path cwd = fs::current_path();

  if(fs::path(filePath).is_absolute())
  {
    CfilePath = filePath;
  }
  else
  {
    CfilePath = cwd.string() + "/" + filePath;
  }

  file.open(CfilePath);
  if(!file.is_open())
  {
    std::cerr << "\n[->] Could not open file: " << CfilePath;
    exit(1);
  }
  std::string dest = "";
  char c;
  bool lineComment = false;
  bool blockComment = false;
  while (file.get(c))
  {
    if (lineComment)
    {
      if (c == '\n')
      {
        lineComment = false;
        dest.append(1, c);
      }
      continue;
    }

    if (blockComment)
    {
      if (c == '\\')
        blockComment = false;
      else if (c == '\n')
        dest.append(1, c);
      continue;
    }

    if (c == '#')
      lineComment = true;
    else if (c == '/')
      blockComment = true;
    else
      dest.append(1, c);
  }
  replaceStr(dest, "  ", "\n");
  replaceStr(dest, " ", "\n");
  replaceStr(dest, "\t", "\n");
  replaceStr(dest, ":", "\n:");
  replaceStr(dest, ",", "\n,");
  replaceStr(dest, "\n\n", "\n");
  replaceStr(dest, "/", "");
  replaceStr(dest, "\\", "");
  replaceStr(dest, "#", "");
  return dest;
}