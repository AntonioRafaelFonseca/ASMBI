# ASMBI

Assembly By an Idiot was made because MARS lacked decent support for input keys and MMIO, making game development difficult. The display was also cumbersome to work with, and MARS was written in Java wich is awfull.

### Compiling

`python compile.py`

*or*

`g++ .\src\chip.cpp .\src\main.cpp -o main.exe -IC:\raylib\raylib\src -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm`

### Executing

`.\main <target file> [/D] [/debug]`

#### Execution Args:
* `[/D]` Display
* `[/debug]` Debug mode. Clears the console and prints an indexed table of source tokens, their token types, and their source text before execution. The program's normal output is printed after the table.

### Registers

You have 16 registers available from `r0` to `rf`.

* `r0`: Opcode register used for syscalls.
* `r1` to `r4`: Argument registers for syscalls.
* `r5` to `rb`, `rd`, `re`, `rf`: General purpose registers.
* `rc`: Control register where the LSB clears the display.

### Instruction Set

* `wr [dest_reg], [value or source_reg]`: Writes a value or the contents of a register into the destination register. Example: `wr r1, 10` or `wr r1, r2`.
* `lbl [lbl_name]`: Defines a label position in your code.
* `goto [lbl_name]`: Unconditional jump to a label.
* `geq [reg1], [reg2], [lbl_name]`: Jumps to the label if `reg1` equals `reg2`.
* `glt [reg1], [reg2], [lbl_name]`: Jumps if `reg1` is less than `reg2`.
* `glte [reg1], [reg2], [lbl_name]`: Jumps if `reg1` is less than or equal to `reg2`.
* `ggt [reg1], [reg2], [lbl_name]`: Jumps if `reg1` is greater than `reg2`.
* `ggte [reg1], [reg2], [lbl_name]`: Jumps if `reg1` is greater than or equal to `reg2`.
* `add [dest_reg], [value or source_reg]`: Adds a value or register to the destination register.
* `sub [dest_reg], [value or source_reg]`: Subtracts a value or register from the destination register.
* `mult [dest_reg], [value or source_reg]`: Multiplies the destination register by a value or register.
* `div [dest_reg], [value or source_reg]`: Divides the destination register by a value or register.
* `syscall`: Executes an operation based on the value stored in `r0`.

Arithmetic operations are assembly instructions, not syscalls. For example, `add r1, r2` updates `r1` to `r1 + r2`.

### Syscalls

To run a syscall, place the operation code into `r0`, set your arguments in `r1` through `r4`, and call `syscall`.

* `0` (CEASE_PROGRAM): Stops execution and exits the program.
* `1` (SAVE_MEMORY): Saves the two least significant bytes from `r1` into the memory address stored in `r2`.
* `2` (READ_MEMORY): Reads memory from the address stored in `r2` into `r1`.
* `3`–`6`: Reserved.
* `7` (AND): Performs a bitwise AND between `r2` and `r3`, storing the result in `r1`.
* `8` (OR): Performs a bitwise OR between `r2` and `r3`, storing the result in `r1`.
* `9` (XOR): Performs a bitwise XOR between `r2` and `r3`, storing the result in `r1`.
* `10` (NOT): Performs a bitwise NOT on `r2`, storing the result in `r1`.
* `11` (SHIFT_L): Shifts `r2` left by `r3` bits, storing the result in `r1`.
* `12` (SHIFT_R): Shifts `r2` right by `r3` bits, storing the result in `r1`.
* `13` (PRINT_INT): Prints the integer value of `r1` to the console without adding a newline.
* `14` (PRINT_CHAR): Prints the ASCII character corresponding to the least significant byte of `r1`.

Example program that adds two values, prints the result, and exits:

```asm
lbl main:
wr r1, 5
wr r2, 7
add r1, r2
wr r0, 13
syscall
wr r0, 0
syscall
```