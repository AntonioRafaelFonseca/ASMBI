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
  file.open(filePath);
  if(!file.is_open())
  {
    std::cerr << "\n[->] Could not open file: " << filePath;
    exit(1);
  }
  std::string dest = "";
  char c;
  bool stoped = 0;
  bool multilineStopped = 0;
  while (file.get(c))
  {
    if(c=='#') stoped = 1;
    if(c=='\n') stoped = 0;
    if(c=='/') multilineStopped = 1;
    if(c==92) multilineStopped = 0;

    if(!(stoped || multilineStopped)) dest.append(1, c);
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