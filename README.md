# ProxOS

Just a fun side project.

# Project structure
| Subproject | purpose | directory |
| ----- | ----- | ----- |
| ProxVM | the bytecode VM that PRX programs run on in the ProxOS | /src/vm |
| PrASM | the simple assembler that compiles PrASM to PRX bytecode | /src/asm |
| Prox CLI | the host tool for driving the VM and the assembler from a normal OS | /src/cli |
| ProxKernel | the OS kernel for ProxOS | /kernel |

The public headers live in /include/prox, the test suites live in /tests, and example PrASM programs live in /examples.

# Building
Everything builds with CMake, and everything it generates lands in /build so the source tree stays clean.

```
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

| Target | kind | output |
| ----- | ----- | ----- |
| prox_vm | static library | build/lib/libproxvm.a |
| prox_asm | static library | build/lib/libproxasm.a |
| prox | executable | build/bin/prox |
| prox_vm_test | test executable | build/bin/prox_vm_test |
| prox_asm_test | test executable | build/bin/prox_asm_test |

Options are set at configure time with -D:

| Option | default | description |
| ----- | ----- | ----- |
| PROX_BUILD_CLI | ON | build the prox host driver |
| PROX_BUILD_TESTS | ON | build the test suites |
| PROX_VM_DEBUG | ON | compile the VM with diagnostic error output |
| PROX_WERROR | OFF | treat compiler warnings as errors |

The CLI and the tests both need a host operating system, so they default to OFF when cross compiling for the kernel. PROX_VM_DEBUG changes the error_out macro in the public header, so the library and anything including it have to agree on it.

# The Prox CLI
The CLI runs the VM and the assembler from a non-ProxOS system (i.e. windows, linux, macos, etc.):

```
prox vm FILE.prx
prox asm FILE.prasm
```

Both the VM and the assembler have low quality error reporting on purpose, since they are meant to be embedded libraries. Make sure the input is correct going in, otherwise the errors coming out will not be much help.

# Examples
/examples holds PrASM programs to run the CLI against:

| File | description |
| ----- | ----- |
| hello.prasm | the smallest useful program, prints hi |
| demo.prasm | prints a banner, counts down with a branch, calls a sub, then stores and loads from the heap |

# Executable/Language Formats and Files
## PrASM, the assembly language
There are two primary syntaxes to remember:
1. Instruction (most common): `OP_ID [args]`
    * there will either be 0, 1, or 2 args (NOP and RET must have zero args)
    * the argument values are as follows:
        * register: R[0-10]
        * integer: any 32 bit integer value
        * label id: any 32 bit integer value
2. Label definition: `LBL id_number:`

Anything after a `;` is a comment and runs to the end of that line. Spaces, tabs, newlines, and colons all just separate tokens, so the layout is up to you.

A small program that prints hi:

```
; prints hi and a newline
SET R0 104
PRC R0
SET R0 105
PRC R0
SET R0 10
PRC R0
```

The assembler is a near 1:1 translation, the only thing it adds on its own is the header and the padding int after NOP and RET. Writing that padding out yourself (i.e. `NOP 0`) emits an extra int and shifts everything after it, so leave those two bare.

## PRX, the bytecode binary format and the VM
### The file
**executable.prx OR executable**

PRX is what runs on the VM, it is not direct binary but rather a bytecode binary. Every PRX program has a short header: 

| 32 Bit Int Index (byte index x 4) | value | description |
| ----- | ----- | ----- |
| 0 | 80 (P) | magic |
| 1 | 82 (R) | magic |
| 2 | 79 (O) | magic |
| 3 | 88 (X) | magic |
| 4 | Any INT | label count |


### The PRX VM operators and memory (stack, heap, registers)
**operators that take zero arguments still need a following int padding (every instruction is a minimum of 2 ints)**

**operators that take two arguments have the two flag (0x80) set on top of the base id below, so SET is really 0x81 and JMZ is really 0x93, which also makes those instructions 3 ints wide**
| ID | Base ID | Args | Description |
| -------- | -------- | -------- | -------- |
| NOP | 0x00 | n/a | placeholder operator |
| SET | 0x01 | R[0-10], integer | sets the register to the provided value |
| INC | 0x02 | R[0-10] | increments the register by one |
| DEC | 0x03 | R[0-10] | decrements the register by one |
| LOD | 0x04 | R[0-10], R[0-10] | loads the value pointed to by arg1 into arg2 |
| STR | 0x05 | R[0-10], integer | stores the value of arg2 into the memory pointed to by arg1 |
| PRI | 0x06 | R[0-10] | prints the value in arg1 as an integer to stdout |
| PRC | 0x07 | R[0-10] | prints the value in arg1 as a character to stdout |
| REQ | 0x08 | R[0-10], R[0-10] | requests space for arg1's value in integers on the heap, if successful points arg2 at that memory |
| DEL | 0x09 | R[0-10] | frees the memory used by arg1 |
| CPY | 0x10 | R[0-10], R[0-10] | copies the value of arg1 to arg2 |
| LBL | 0x11 | integer | creates a label with id arg1 |
| JMP | 0x12 | integer | jumps to the label with id arg1 |
| JMZ | 0x13 | R[0-10], integer | jumps to the label with id arg2 only if arg1's value is zero |
| CAL | 0x14 | integer | adds a frame to the stack, copies over the current register values to the new frame, sets the return address, then jumps to the label |
| RET | 0x15 | n/a | returns to the previous stack frame (removing the current one), and sets the instruction pointer to the return address of the current stack frame |
| RTR | 0x16 | R[0-10] | same as return, but also sets the incoming stack frame's return register to hold the value at arg1 |

Note that STR takes an integer for its second argument and not a register, so it can only write constants into the heap. LOD and REQ both take registers for both arguments.

The VM has a simple stack/register flow:
- Each stack frame has 10 registers + 1 return register that holds the return value from the previous stack frame
- The most current stack frame is always the first (LIFO)
- Branching can be done with labels, it is pretty much a GOTO system with the ability for conditional GOTO jumps
- To advance the stack frame with a branch (i.e. get a fresh stack frame), use the CAL operator
    - This will add a new stack frame, and then copy over the register values (not the return register) from the current stack frame and then jump to the corresponding label
    - Since registers are copied, this is the best way to emulate passing "arguments"
- Normal jumps do not advance the stack frame
- Calling a return operator will step back the stack frame (pop out the front), then set the instruction pointer to the new stack frame's return address
    - If you call the return with register operator, it will also copy the current stack frame's value at the specified register into the new stack frame's return register
- Labels are collected in a first pass before anything runs, so a jump can point at a label further down the program
- Every program is given a set heap (right now its 1024 32 bit integers)
    - It is a first fit allocator, it walks the chunk list and takes the first gap wide enough, otherwise it appends onto the end
    - If a request does not fit, the heap is compacted and the request is retried once before it gives up
    - Memory is still manually managed though, users must claim and free memory
