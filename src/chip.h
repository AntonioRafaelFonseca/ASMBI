#pragma once

#include <array>
#include <cstddef>

typedef unsigned char byte;
typedef unsigned short halfword;
typedef unsigned int word;

struct MEMORY
{
  std::array<halfword, 0x10000> addrs{};
  halfword& at(word i);
};

struct REGISTERS
{
  std::array<word, 0x10> regs{};
  word& at(int i);
};

enum REGISTER_INDEX
{
  r0, // opcode register

  r1, //
  r2, //    syscall args in order
  r3, //
  r4, //

  r5, //    gpr
  r6, //
  r7, //
  r8, //
  r9, //

  ra, //
  rb, //

  rc, // Enables the background color for zero-valued display pixels

  rd, //
  re, // general-purpose register
  rf, //
};

enum class SYSCALL_RESULT
{
  CONTINUE,
  HALT,
  ERROR
};

class chip
{
  private:
  public:
  MEMORY mem;
  REGISTERS regs;
  void writeReg(REGISTER_INDEX reg, word value);
  word readReg(REGISTER_INDEX reg);
  SYSCALL_RESULT syscall();
  void tick();
};

enum OP_CODES
{
  CEASE_PROGRAM,

  SAVE_MEMORY,
  READ_MEMORY,

  AND,
  OR,
  XOR,
  NOT,

  SHIFT_L,
  SHIFT_R,

  PRINT_INT,
  PRINT_CHAR
};