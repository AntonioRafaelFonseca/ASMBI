#include <iostream>
#include <cstdlib>
#include "chip.h"

word& REGISTERS::at(int i)
{
  if (i >= 0 && i < static_cast<int>(this->regs.size()))
    return this->regs[static_cast<size_t>(i)];

  std::cerr << "\n[ERROR] Invalid Register Address:  " << i;
  exit(EXIT_FAILURE);
}
halfword& MEMORY::at(word i)
{
  if (i < this->addrs.size()) return this->addrs[i];
  std::cerr << "\n[ERROR] Invalid Memory Address:  " << i;
  exit(EXIT_FAILURE);
}

void chip::writeReg(REGISTER_INDEX reg, word value)
{
  if(reg == rr) return;
  this->regs.at(static_cast<int>(reg)) = value;
}

word chip::readReg(REGISTER_INDEX reg)
{
  if(reg == rr) regs.at(rr) = rand() % 0xffffffff;
  return this->regs.at(static_cast<int>(reg));
}

void chip::tick() {this->mem.at(0)++;}

SYSCALL_RESULT chip::syscall()
{
  const word code = this->regs.at(r0);

  switch (code)
  {
  case CEASE_PROGRAM:
    {
      std::cout << "\n Program Finished\n";
      return SYSCALL_RESULT::HALT;
    }

  case SAVE_MEMORY:
  {
    halfword value = readReg(r1); // reads only the 2 ls bytes from the r1 reg
    word addr = readReg(r2);
    this->mem.at(addr) = value;
    break;
  }
  case READ_MEMORY:
  {
    word addr = readReg(r2);
    this->regs.at(r1) = this->mem.at(addr);
    break;
  }

  case AND:
  {
    this->regs.at(r1) = this->regs.at(r2) & this->regs.at(r3);
    break;
  }
  case OR:
  {
    this->regs.at(r1) = this->regs.at(r2) | this->regs.at(r3);
    break;
  }
  case XOR:
  {
    this->regs.at(r1) = this->regs.at(r2) ^ this->regs.at(r3);
    break;
  }
  case NOT:
  {
    this->regs.at(r1) = ~this->regs.at(r2);
    break;
  }

  case SHIFT_L:
  {
    word shift = this->regs.at(r3);
    if (shift >= sizeof(word) * 8)
    {
      std::cerr << "\n[ERROR] Left shift amount must be less than "
                << sizeof(word) * 8 << '\n';
      return SYSCALL_RESULT::ERROR;
    }
    this->regs.at(r1) = this->regs.at(r2) << shift;
    break;
  }
  case SHIFT_R:
  {
    word shift = this->regs.at(r3);
    if (shift >= sizeof(word) * 8)
    {
      std::cerr << "\n[ERROR] Right shift amount must be less than "
                << sizeof(word) * 8 << '\n';
      return SYSCALL_RESULT::ERROR;
    }
    this->regs.at(r1) = this->regs.at(r2) >> shift;
    break;
  }


  case PRINT_INT:
  {
    std::cout << readReg(r1) << std::flush;
    break;
  }
  case PRINT_CHAR:
  {
    std::cout << static_cast<char>(readReg(r1) & 0xff) << std::flush;
    break;
  }
  default:
    std::cerr << "\n[ERROR] Invalid syscall code: "
              << this->regs.at(r0) << '\n';
    return SYSCALL_RESULT::ERROR;
  }
  return SYSCALL_RESULT::CONTINUE;
}