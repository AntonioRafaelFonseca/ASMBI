#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct LBL
{
  std::string name;
  int pcIndex;
};

std::vector<LBL> GlobalLabels;

int findLabel(std::string s)
{
  for(auto l : GlobalLabels)
  {
    if (l.name == s) return l.pcIndex;
  }
  std::cerr << "invalid label " << s;
  exit(1);
  return INT_MAX;
}

void replaceStr(std::string& c, const std::string a, const std::string b) {
  if (a.empty()) return;
  size_t pos = c.find(a);
  while (pos != std::string::npos) {
      c.replace(pos, a.length(), b);
      pos = c.find(a, pos + b.length());
  }
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
    dest.append(1, c);
  }
  replaceStr(dest, "  ", "\n");
  replaceStr(dest, " ", "\n");
  replaceStr(dest, "\t", "\n");
  replaceStr(dest, ":", "\n:");
  replaceStr(dest, ",", "\n,");
  replaceStr(dest, "\n\n", "\n");
  return dest;
}