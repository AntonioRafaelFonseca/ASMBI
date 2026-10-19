# ASMBI

**Assembly By an Idiot** is a small assembly-like emulator written in C++. It includes a simple instruction set, a 16-bit memory space, console syscalls, and an optional Raylib display with memory-mapped input and clock values.

## Build

With Python and `g++` available, build from the repository root:

```powershell
python .\compile.py
```

Or compile directly (adjust the Raylib paths if Raylib is installed elsewhere):

```powershell
g++ .\src\chip.cpp .\src\main.cpp -o main.exe `
  -IC:\raylib\raylib\src -LC:\raylib\raylib\src `
  -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Run

```powershell
.\main.exe <program.asmbi> [/D] [/debug]
```

- `/D` opens the 128 × 64 display window, enlarged 3×.
- `/debug` clears the console and shows a numbered table of source tokens and their token types before the program runs.
- The flags can be supplied in either order.

## Registers

The emulator has sixteen 32-bit registers, named `r0` through `rf` (hexadecimal register names).

| Registers | Use |
| --- | --- |
| `r0` | Syscall number |
| `r1`–`r4` | Syscall arguments and results |
| `r5`–`rb`, `rd`–`rf` | General-purpose registers |
| `rc` | Display control; a nonzero value makes the renderer use the background color for pixels |

`re` and `rf` are ordinary general-purpose registers. Register `rc` has the display behavior described above.

## Instructions

Each instruction is written using its mnemonic, followed by operands separated by commas. Operands for arithmetic and conditional branches can be integer literals or registers where applicable.

| Instruction | Example | Behavior |
| --- | --- | --- |
| `wr` | `wr r1, 42` | Writes a literal or another register's value to the destination register. |
| `add` | `add r1, r2` | Adds the second operand to the destination register. |
| `sub` | `sub r1, 2` | Subtracts the second operand from the destination register. |
| `mult` | `mult r1, r2` | Multiplies the destination register by the second operand. |
| `div` | `div r1, 2` | Divides the destination register by the second operand using integer division. |
| `lbl` | `lbl loop:` | Defines a label that branches can target. |
| `goto` | `goto loop` | Jumps unconditionally to a label. |
| `geq` | `geq r1, r2, same` | Jumps if the operands are equal. |
| `glt` | `glt r1, r2, less` | Jumps if the first operand is less than the second. |
| `glte` | `glte r1, r2, done` | Jumps if the first operand is less than or equal to the second. |
| `ggt` | `ggt r1, r2, more` | Jumps if the first operand is greater than the second. |
| `ggte` | `ggte r1, r2, done` | Jumps if the first operand is greater than or equal to the second. |
| `slp` | `slp 100` | Pauses for the specified number of milliseconds. |
| `syscall` | `syscall` | Runs the operation selected by `r0`. |

Execution starts at the `main` label. Put setup instructions after `lbl main:` if they must run when the program starts. `#` begins a comment that continues to the end of the line.

## Syscalls

Set the syscall number in `r0`, provide any arguments in the registers below, and execute `syscall`.

| `r0` | Name | Behavior |
| ---: | --- | --- |
| `0` | `CEASE_PROGRAM` | Ends the program. |
| `1` | `SAVE_MEMORY` | Writes the low 16 bits of `r1` to the memory address in `r2`. |
| `2` | `READ_MEMORY` | Reads the memory word at the address in `r2` into `r1`. |
| `3` | `AND` | Stores `r2 & r3` in `r1`. |
| `4` | `OR` | Stores `r2 \| r3` in `r1`. |
| `5` | `XOR` | Stores `r2 ^ r3` in `r1`. |
| `6` | `NOT` | Stores the bitwise complement of `r2` in `r1`. |
| `7` | `SHIFT_L` | Stores `r2` shifted left by `r3` bits in `r1`. |
| `8` | `SHIFT_R` | Stores `r2` shifted right by `r3` bits in `r1`. |
| `9` | `PRINT_INT` | Prints the integer value in `r1` without adding a newline. |
| `10` | `PRINT_CHAR` | Prints the character represented by the low byte of `r1`. |

The arithmetic instructions (`add`, `sub`, `mult`, and `div`) are separate from the syscall numbers.

## MEMORY

Memory contains 65,536 addressable words. Each word is 16 bits wide. The clock, key input, and display use reserved addresses within this same memory space.

### Main memory and system MMIO

| Address | Name | Description |
| --- | --- | --- |
| `0x0000` | Clock counter | Increments once per emulator instruction step. It is a 16-bit word and wraps around on overflow. |
| `0x0001` | Key input | Updated from Raylib's key queue on each instruction step; contains the key code, or `0` when no key is queued. |
| `0x0002`–`0xBFFC` | General memory | Available for program data. |
| `0xDFFE`–`0xFFFF` | General memory | Also addressable as program data; these locations are outside the visible framebuffer. |

Programs can read or write memory with the `READ_MEMORY` and `SAVE_MEMORY` syscalls. The display window is optional; without `/D`, the memory and console syscalls are still available.

### Display memory map

Start the emulator with `/D` to open the display. Pixels use 16-bit RGB565 color values: 5 red bits, 6 green bits, and 5 blue bits.

| Address | Name | Description |
| --- | --- | --- |
| `0xBFFD` | Background color | RGB565 color used for the display background when `rc` is nonzero. |
| `0xBFFE`–`0xDFFD` | Framebuffer | 8,192 words for the 128 × 64 visible pixels, in row-major order. The first address is the top-left pixel. |
| `0xDFFE`–`0xFFFD` | Outside visible framebuffer | The renderer scans this higher range but clips it because it is beyond the 64 visible rows. It remains part of addressable memory. |

The display is refreshed as the program executes. Pixel `(x, y)` is stored at `0xBFFE + y * 128 + x`.

## Example

This program adds two values, prints the result, and exits:

```asm
lbl main:
wr r1, 5
wr r2, 7
add r1, r2
wr r0, 9
syscall
wr r0, 0
syscall
```
