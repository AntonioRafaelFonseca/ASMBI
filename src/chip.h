#pragma once

typedef unsigned char byte;
typedef unsigned short halfword;
typedef unsigned int word;

struct MEMORY
{
  halfword *addrs = new halfword[0x10000];
  halfword& at(int i);
};

struct REGISTERS
{
  word regs[0x10] = { 0 };
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

  rc, // lsb is the clear displ bit  msb is the carry flag or smth

  rd, //
  re, //still gpr but with letters
  rf,  //
};

class chip
{
  private:
  public:
  MEMORY mem;
  REGISTERS regs;
  void writeReg(REGISTER_INDEX reg, word value);
  word readReg(REGISTER_INDEX reg);
  void syscall();
  void tick();
  ~chip();
};

enum OP_CODES
{
  CEASE_PROGRAM,
  SAVE_MEMORY,
  READ_MEMORY,
  ADD,
  SUB,
  PRINT_INT,
  PRINT_CHAR
};