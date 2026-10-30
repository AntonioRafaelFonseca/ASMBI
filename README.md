# ASMBI

**Assembly By an Idiot** is a small assembly-like emulator written in C++. It provides a compact assembly language, sixteen 32-bit registers, 65,536 words of 16-bit memory, console and bitwise syscalls, a memory-mapped instruction counter, and an optional Raylib display with keyboard input.

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

- `.\asmbi.cmd <program.asmbi> [/D]` runs the executable beside the command file and enables `/debug`.
- `/D` opens the 128 × 64 display window, enlarged 4×.
- `/debug` shows each compiled output token's index, type, value, and source text, along with the addresses assigned to labels. It also enables Raylib's trace messages.
- Supply exactly one program file. The `/D` and `/debug` flags can appear before or after it, in either order.
- If the program file is missing, an option is unknown, or more than one file is supplied, the emulator prints an error and usage information.
- Argument errors exit with status `2`; execution and input errors return a nonzero status.

For example, both of these commands enable display and debug mode:

```powershell
.\main.exe .\main.asmbi /D /debug
.\main.exe /debug /D .\main.asmbi
```

## Registers

The emulator has sixteen 32-bit unsigned registers, named `r0` through `rf` (hexadecimal register names). Values wrap modulo 2^32 during arithmetic.

| Registers | Use |
| --- | --- |
| `r0` | Syscall number |
| `r1`–`r4` | Syscall arguments and results |
| `r5`–`rb`, `rd`–`rf` | General-purpose registers |
| `rc` | Display control; when nonzero, zero-valued pixels are drawn using the background color |

Register names use a lowercase `r` followed by one hexadecimal digit (`r0`–`r9`, `ra`–`rf`).

## Instructions

Each instruction is written using its mnemonic, followed by operands separated by commas. Integer literals can be decimal, hexadecimal (`0x`), or binary (`0b`), and must fit in 32 bits. Arithmetic and branch operands can be registers or integer literals.

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
| `gneq` | `gneq r1, r2, different` | Jumps if the operands are not equal. |
| `glt` | `glt r1, r2, less` | Jumps if the first operand is less than the second. |
| `glte` | `glte r1, r2, done` | Jumps if the first operand is less than or equal to the second. |
| `ggt` | `ggt r1, r2, more` | Jumps if the first operand is greater than the second. |
| `ggte` | `ggte r1, r2, done` | Jumps if the first operand is greater than or equal to the second. |
| `slp` | `slp 100` | Pauses for the specified number of milliseconds. The duration can be a literal or register. |
| `syscall` | `syscall` | Runs the operation selected by `r0`. |

Execution starts at the required `main` label, regardless of where it appears in the source file. Labels use the form `lbl name:` and must be unique. Put setup instructions after `lbl main:` if they must run when the program starts. `#` begins a comment that continues to the end of the line. A block comment starts with `/` and continues through the next `\`.

Conditional branches compare unsigned 32-bit values. Arithmetic wraps modulo 2^32. The tokenizer validates instruction operands, label declarations and references, register names, and integer literals before execution. Invalid input is reported to the console; runtime errors such as division by zero, an invalid syscall, or an invalid shift stop execution with a failing process exit code.

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

Memory reads return a 16-bit word in the 32-bit destination register; memory writes store the low 16 bits of the source register. Bitwise syscall results are 32-bit. The arithmetic instructions (`add`, `sub`, `mult`, and `div`) are separate from the syscall numbers.

Syscall `0` ends the emulator loop cleanly so display resources are released. A syscall number outside `0`–`10`, or a bit-shift amount of 32 or more, reports an error and stops execution. Division by zero also reports an error and stops the current program. Integer and character output is flushed immediately; neither print syscall adds a newline.

## MEMORY

Memory contains 65,536 addressable words. Each word is 16 bits wide. The clock, key input, and display use reserved addresses within this same memory space.

### Main memory and system MMIO

| Address | Name | Description |
| --- | --- | --- |
| `0x0000` | Clock counter | A zero-initialized 16-bit value that increments once per executed instruction and wraps around on overflow. |
| `0x0001` | Key input | With the display enabled, polled each instruction; contains the first matching held-key code or `0` if none of the supported keys is held. |
| `0x0002`–`0xBFFC` | General memory | Available for program data. |
| `0xFFFE`–`0xFFFF` | General memory | Addressable program data not scanned by the display renderer. |

All 65,536 addresses (`0x0000`–`0xFFFF`) are valid, and memory starts at zero. The clock increments once for each executed instruction and wraps as a 16-bit value. Programs can read or write memory with `READ_MEMORY` and `SAVE_MEMORY`. Without `/D`, display drawing and update instructions are ignored and the key input remains `0`.

### Display memory map

Start the emulator with `/D` to open the display, which is targeted at 60 FPS. Pixels use 16-bit RGB565 color values: 5 red bits, 6 green bits, and 5 blue bits.

| Address | Name | Description |
| --- | --- | --- |
| `0xBFFD` | Background color | RGB565 color used for zero-valued pixels when `rc` is nonzero. |
| `0xBFFE`–`0xDFFD` | Framebuffer | 8,192 words for the 128 × 64 visible pixels, in row-major order. The first address is the top-left pixel. |
| `0xDFFE`–`0xFFFD` | Outside visible framebuffer | Still scanned by the renderer, but clipped because these addresses map beyond the visible 64 rows. They remain ordinary addressable memory. |

Pixel `(x, y)` is stored at `0xBFFE + y * 128 + x`. Pixel words use RGB565 color: 5 red bits, 6 green bits, and 5 blue bits. `DRAW` reads framebuffer memory, updates the texture, and presents it. `UPDATE` presents the current texture without rereading framebuffer memory; issue `DRAW` after changing pixels to refresh them. With `rc` set to nonzero, zero-valued pixels show the background color from `0xBFFD`; otherwise zero-valued pixels are skipped. Consequently, clearing a pixel to zero while `rc` is zero may leave its previous color visible until the image buffer is reset (for example, by enabling background rendering); this is a current renderer limitation. Closing the display window stops execution. Supported key codes are digits `0`–`9`, letters `A`–`F`, space, and the arrow keys (Raylib key codes).

## Debugging and errors

Use `/debug` to inspect the tokenizer's generated instruction stream and resolved labels before execution. The emulator reports invalid registers, malformed instructions and labels, out-of-range literals, invalid syscalls, invalid shift amounts, and division by zero. A missing or unreadable source file is also reported. Parser/input errors terminate with a nonzero process status; invalid command-line arguments use status `2`.

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
