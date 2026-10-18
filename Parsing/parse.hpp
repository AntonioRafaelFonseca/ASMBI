#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>

struct LBL
{
  std::string name;
  int pcIndex;
};

std::vector<LBL> GlobalLabels;

int findLabel(std::string s, int tokenLocIndex)
{
  for(auto l : GlobalLabels)
  {
    if (l.name == s) return l.pcIndex;
  }
  if(s=="main")
  {
    std::cerr << "[ERROR] No main label found suggest adding \"lbl main\" at the start of the program";
    exit(1);
  }
  else
  {
    std::cerr << "[ERROR] Token: " << std::bitset<16>(tokenLocIndex) << " - " << tokenLocIndex << " Invalid label '" << s << "'";
    exit(1);
  }
  return INT_MAX;
}

std::string replaceStr(std::string& c, const std::string a, const std::string b) {
  if (a.empty()) return "";
  size_t pos = c.find(a);
  while (pos != std::string::npos) {
      c.replace(pos, a.length(), b);
      pos = c.find(a, pos + b.length());
  }
  return c;
}

std::string readsimplifyCode(const char* filePath)
{
  std::ifstream file;
  file.open(filePath);
  if(!file.is_open())
  {
    std::cerr << "[->] Could not open file: " << filePath;
    exit(1);
  }
  std::string dest = "";
  char c;
  bool stoped = 0;
  while (file.get(c))
  {
    if(c=='#') stoped = 1;
    if(c=='\n') stoped = 0;
    if(!stoped)  dest.append(1, c);
  }
  replaceStr(dest, "  ", "\n");
  replaceStr(dest, " ", "\n");
  replaceStr(dest, "\t", "\n");
  replaceStr(dest, ":", "\n:");
  replaceStr(dest, ",", "\n,");
  replaceStr(dest, "\n\n", "\n");
  return dest;
}