#include <iostream>
#include "chip.h"

word& REGISTERS::at(int i)
{
  if (i<16 && i>=0) return this->regs[i];
  std::cerr << "[->] ERROR: Invalid Register Address:  " << i;
  exit(EXIT_FAILURE);
}
halfword& MEMORY::at(int i)
{
  if (i<0xffff && i>=0) return this->addrs[i];
  std::cerr << "[->] ERROR: Invalid Memory Address:  " << i;
  exit(EXIT_FAILURE);
}

chip::~chip()
{
  delete this->mem.addrs;
}

void chip::writeReg(REGISTER_INDEX reg, word value)
{
  byte index = static_cast<byte>(reg);
  this->regs.at(index) = value;
}

word chip::readReg(REGISTER_INDEX reg)
{
  return this->regs.at(static_cast<byte>(reg));
}
void chip::tick() {this->mem.at(0)++;}

void chip::syscall()
{
  OP_CODES code = static_cast<OP_CODES>(this->regs.at(0));

  switch (code)
  {
  case CEASE_PROGRAM:
    {
      std::cout << "\n Program Finished";
      exit(0);
    }
  case SAVE_MEMORY:
  {
    halfword value = readReg(r1) & 0x7fff; // reads only the 2 ls bytes from the r1 reg
    word addr = readReg(r2) & 0x7fff;
    this->mem.at(addr) = value;
    break;
  }

  case READ_MEMORY:
  {
    word addr = readReg(r2) & 0x7fff;
    this->regs.at(r1) = this->mem.at(addr);
    break;
  }
  case ADD:
  {
    this->regs.at(r1) = this->regs.at(r2) + this->regs.at(r3);
    break;
  }
  case PRINT_INT:
  {
    std::cout << (int)readReg(r1);
    break;
  }
  case PRINT_CHAR:
  {
    std::cout << static_cast<char>(readReg(r1) & 0xff);
    break;
  }
  default:
    break;
  }
}