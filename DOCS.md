# ASMBI

**Assembly By an Idiot** is a small assembly-like emulator written in C++. It provides a compact assembly language, sixteen 32-bit registers, 65,536 words of 16-bit memory, console and bitwise syscalls, a memory-mapped instruction counter, and an optional Raylib display with keyboard input.

This document is the detailed reference for building and using the emulator and for writing `.asmbi` programs. The shorter [README](./README.md) provides the project overview.

## Contents

- [Requirements and build](#requirements-and-build)
- [Run the emulator](#run-the-emulator)
- [Assembly source format](#assembly-source-format)
- [Registers and values](#registers-and-values)
- [Language instructions](#language-instructions)
- [Syscalls](#syscalls)
- [Memory and MMIO](#memory-and-mmio)
- [Display, DRAW, and UPDATE](#display-draw-and-update)
- [Keyboard input](#keyboard-input)
- [Debugging and errors](#debugging-and-errors)
- [Complete examples](#complete-examples)

## Requirements and build

The project is written in C++ and uses Raylib. On Windows, the included build script expects Python, `g++`, and Raylib installed at `C:\raylib\raylib\src`.

Build from the repository root:

```powershell
python .\compile.py
```

The script compiles the C++ sources into `main.exe` and links Raylib and the Windows OpenGL, GDI, and multimedia libraries.

Alternatively, compile directly, adjusting the include and library paths to match your Raylib installation:

```powershell
g++ .\src\chip.cpp .\src\main.cpp -o main.exe `
  -IC:\raylib\raylib\src -LC:\raylib\raylib\src `
  -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Run the emulator

The program accepts exactly one assembly source path and the optional `/D` and `/debug` switches. Switches can appear before or after the path:

```powershell
.\main.exe .\main.asmbi
.\main.exe .\main.asmbi /D
.\main.exe /debug .\main.asmbi /D
```

| Option | Effect |
| --- | --- |
| `/D` | Initializes the Raylib window at 128 x 64 pixels, displayed at 4x scale and targeted at 60 FPS. |
| `/debug` | Prints the compiled token stream and label addresses before execution, and enables Raylib trace logging. |

An omitted source path, unknown option, or second source path is rejected with usage information and exit status `2`. Runtime and source errors return a nonzero status. A successful halt or normal completion returns status `0`.

## Assembly source format

### Instructions and separators

Each instruction uses its mnemonic followed by the required operands. Commas separate operands, and a label declaration ends with a colon:

```asm
lbl main:
wr r1, 12
add r1, 3
```

Spaces, tabs, and line breaks separate tokens. Mnemonics are case-sensitive: ordinary instructions are lowercase, while the display instructions are uppercase `DRAW` and `UPDATE`.

### Comments

- `#` starts a single-line comment, ending at the next newline.
- `/` starts a block comment, and `\` ends it.

Example:

```asm
# Set up the program
lbl main:
wr r1, 1

/ This text is ignored
  until the closing backslash \
syscall
```

The slash and backslash delimiters are comment syntax, not division operators.

### Integer literals

Unsigned integer values can be written in decimal, lowercase-prefixed hexadecimal, or lowercase-prefixed binary:

```asm
wr r1, 255
wr r2, 0xff
wr r3, 0b11111111
```

All literal forms must fit in an unsigned 32-bit value (`0` through `4294967295`). Hexadecimal digits may use either case; the prefixes are `0x` and `0b`. Negative literals are not supported. Arithmetic results are unsigned and wrap modulo 2^32.

### Labels and entry point

Labels use `lbl name:` and can be used as destinations for `goto` and conditional branches. They must be unique. Execution begins at the required `main` label, not necessarily at the first line of the source file. Instructions before `lbl main:` are not executed unless reached by a branch.

Label names are case-sensitive. Do not use a reserved mnemonic, a register spelling, or a decimal number as a label name. In particular, names beginning with lowercase `r` are parsed as register tokens and cannot be used as labels.

## Registers and values

The emulator has sixteen unsigned 32-bit registers. Register names consist of lowercase `r` followed by one hexadecimal digit:

| Registers | Purpose |
| --- | --- |
| `r0` | Syscall selector; read by `syscall`. |
| `r1`-`r4` | Syscall arguments and results. Individual calls use only the registers listed in the syscall table. |
| `r5`-`rb` | General-purpose registers. |
| `rc` | Display control. When nonzero, zero-valued framebuffer words are rendered using the background color. |
| `rd`-`rf` | General-purpose registers. |

Registers are initialized to zero. Arithmetic instructions update the first register operand and wrap on overflow or underflow. Memory words are 16-bit even though registers and arithmetic values are 32-bit.

## Language instructions

All operands in the instruction table are required. For `wr`, arithmetic, and `slp`, an operand may be either a register or an integer literal as indicated. Branch comparison values may also be registers or literals.

| Instruction | Form | Behavior |
| --- | --- | --- |
| `wr` | `wr register, value` | Copies a register or literal into the destination register. |
| `add` | `add register, value` | Adds the value to the destination register. |
| `sub` | `sub register, value` | Subtracts the value from the destination register. |
| `mult` | `mult register, value` | Multiplies the destination register by the value. |
| `div` | `div register, value` | Integer-divides the destination register by the value. Division by zero is an error. |
| `lbl` | `lbl name:` | Defines a unique branch target. Labels do not execute as instructions. |
| `goto` | `goto name` | Jumps unconditionally to a label. |
| `geq` | `geq left, right, name` | Jumps when `left` equals `right`. |
| `gneq` | `gneq left, right, name` | Jumps when `left` does not equal `right`. |
| `glt` | `glt left, right, name` | Jumps when `left` is less than `right`. |
| `glte` | `glte left, right, name` | Jumps when `left` is less than or equal to `right`. |
| `ggt` | `ggt left, right, name` | Jumps when `left` is greater than `right`. |
| `ggte` | `ggte left, right, name` | Jumps when `left` is greater than or equal to `right`. |
| `slp` | `slp milliseconds` | Sleeps for the unsigned millisecond value, supplied as a literal or register. |
| `syscall` | `syscall` | Performs the operation selected by `r0`. |
| `DRAW` | `DRAW` | With `/D`, reads framebuffer memory, updates the texture, and draws it in a Raylib frame. Without `/D`, it is ignored. |
| `UPDATE` | `UPDATE` | With `/D`, draws the already-uploaded texture in a Raylib frame without rereading framebuffer memory. Without `/D`, it is ignored. |

Conditional branches compare unsigned 32-bit values. The branch layout is `mnemonic operand, operand, label`; for example:

```asm
geq r1, 0, finished
goto loop
lbl finished:
```

## Syscalls

To issue a syscall, place its number in `r0`, set its input arguments, then execute `syscall`.

| `r0` | Name | Inputs | Result |
| ---: | --- | --- | --- |
| `0` | `CEASE_PROGRAM` | None | Stops execution cleanly. |
| `1` | `SAVE_MEMORY` | `r1` = value, `r2` = address | Writes the low 16 bits of `r1` to memory at `r2`. |
| `2` | `READ_MEMORY` | `r2` = address | Reads the memory word at `r2` into `r1`. |
| `3` | `AND` | `r2`, `r3` | Stores `r2 & r3` in `r1`. |
| `4` | `OR` | `r2`, `r3` | Stores `r2 \| r3` in `r1`. |
| `5` | `XOR` | `r2`, `r3` | Stores `r2 ^ r3` in `r1`. |
| `6` | `NOT` | `r2` | Stores the 32-bit bitwise complement of `r2` in `r1`. |
| `7` | `SHIFT_L` | `r2` = value, `r3` = shift count | Stores `r2` shifted left by the count in `r3` in `r1`. |
| `8` | `SHIFT_R` | `r2` = value, `r3` = shift count | Stores `r2` shifted right by the count in `r3` in `r1`. |
| `9` | `PRINT_INT` | `r1` = value | Prints the unsigned integer value. |
| `10` | `PRINT_CHAR` | `r1` = value | Prints the low byte of `r1` as a character. |

The print syscalls flush output immediately and do not append a newline. Add a newline explicitly with `PRINT_CHAR` and the newline byte (`10`) if desired.

Shift counts must be between 0 and 31 inclusive. A larger count is reported as an error and stops the program. Any syscall number outside `0`-`10` is also an error. Arithmetic instructions (`add`, `sub`, `mult`, and `div`) are language instructions, not syscall codes.

## Memory and MMIO

Memory consists of 65,536 addressable words, each 16 bits wide. Memory is zero-initialized. All addresses from `0x0000` through `0xFFFF` are valid.

| Address range | Purpose | Details |
| --- | --- | --- |
| `0x0000` | Instruction counter | Increments once for each executed instruction and wraps as a 16-bit value. |
| `0x0001` | Key input | Updated by keyboard polling during execution; see [Keyboard input](#keyboard-input). |
| `0x0002`-`0xBFFC` | General memory | Available for program data. |
| `0xBFFD` | Display background color | RGB565 background value used when `rc` is nonzero. |
| `0xBFFE`-`0xDFFD` | Visible framebuffer | 8,192 words, one for each pixel in the 128 x 64 display. |
| `0xDFFE`-`0xFFFD` | Beyond visible framebuffer | Ordinary memory; also scanned by the renderer but lies outside the visible 64 rows. |
| `0xFFFE`-`0xFFFF` | General memory | Valid ordinary memory, not scanned by the renderer. |

Use `SAVE_MEMORY` and `READ_MEMORY` to write and read words. `SAVE_MEMORY` truncates a register value to its low 16 bits. `READ_MEMORY` zero-extends the 16-bit word into the 32-bit destination register.

## Display, DRAW, and UPDATE

Pass `/D` to initialize the display. The window represents 128 x 64 pixels at 4x scale; the target frame rate is 60 FPS. Each pixel is stored as a 16-bit RGB565 color word:

- Bits 15-11: red (5 bits)
- Bits 10-5: green (6 bits)
- Bits 4-0: blue (5 bits)

The pixel at `(x, y)` is stored at:

```text
0xBFFE + y * 128 + x
```

The visible framebuffer therefore occupies `0xBFFE` through `0xDFFD`. Coordinates start at `(0, 0)` in the top-left corner.

### `DRAW`

`DRAW` reads display memory and updates the texture:

1. If `rc` is nonzero, fills the image buffer with the RGB565 color at `0xBFFD`.
2. Scans memory from `0xBFFE` through `0xFFFD`.
3. Converts nonzero words that map to visible pixels into RGB565 display colors.
4. Uploads the image buffer to the texture and presents it.

When `rc` is zero, the renderer does not clear the image buffer and skips zero-valued framebuffer words. This means that writing zero over a previously drawn pixel may leave the old color visible. Setting `rc` nonzero enables the background fill and makes zero-valued pixels appear as the configured background color.

### `UPDATE`

`UPDATE` presents the current texture, but does not copy memory into the image buffer or upload a new texture. Use `DRAW` after changing framebuffer memory to make those changes visible. `UPDATE` is useful for presenting the existing image without rebuilding it.

Both display instructions are ignored when the emulator is run without `/D`. When a display window is enabled, closing it during a draw or update stops execution and allows display resources to be released.

## Keyboard input

The value at memory address `0x0001` is refreshed during the execution loop. With the display window enabled, the emulator checks for held keys in this order and writes the first matching key code:

| Keys | Value written to `0x0001` |
| --- | --- |
| `0`-`9` | Raylib key codes `48`-`57` |
| `A`-`F` | Raylib key codes `65`-`70` |
| Space | `32` |
| Down arrow | `264` |
| Up arrow | `265` |
| Left arrow | `263` |
| Right arrow | `262` |
| None of the supported keys | `0` |

The listed order determines which key wins if multiple supported keys are held at once. Programs commonly read the value with `READ_MEMORY`, using address `1` in `r2`.

## Debugging and errors

Run with `/debug` to print:

- Each label and its resolved instruction index.
- Each output token's index, token type, numeric value, and originating source text.
- Raylib trace logging.

The tokenizer checks label declarations and duplicates, label references, register spelling, instruction operand count and types, and integer literal format/range before execution. Runtime errors include division by zero, an invalid syscall number, or an invalid shift count. Missing or unreadable source files are also reported. Errors terminate with a nonzero process status; invalid command-line arguments return status `2`.

## Complete examples

### Print an integer and exit

This program adds `5` and `7`, prints `12`, then exits:

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

### Write a pixel and draw it

This program writes a blue RGB565 pixel at `(10, 5)`. Run it with `/D`:

```asm
lbl main:
wr r5, 5
mult r5, 128
add r5, 10
add r5, 49150
wr r1, 31
wr r2, r5
wr r0, 1
syscall
DRAW
wr r0, 0
syscall
```

`0xBFFE` is `49150` in decimal, and RGB565 value `31` is full blue. The pixel write is completed before `DRAW` uploads and presents the framebuffer.

### Loop and conditional branch

```asm
lbl main:
wr r1, 0

lbl loop:
add r1, 1
geq r1, 5, finished
goto loop

lbl finished:
wr r0, 9
syscall
wr r0, 0
syscall
```

The program increments `r1` until it reaches `5`, prints the result, and halts.
