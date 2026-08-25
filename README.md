# ProxOS

Just a fun side project.

# Executable/Language Formats and Files
## PrASM, the assembly language
There are two primary syntaxes to remember:
1. Instruction (most common): `OP_ID [args]`
    * there will either be 0, 1, or 2 args
    * the argument values are as follows:
        * register: R[0-10]
        * integer: any 32 bit integer value
        * label id: any valid identifier that starts with a letter or underscore, than is followed by any letter, number, or underscore (no other characters allowed)
2. Label definition: `LBL label_id:`

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
| ID | Bytecode | Args | Description |
| -------- | -------- | -------- | -------- |
| NOP | 0x00 | n/a | placeholder operator |
| SET | 0x01 | R[0-10], integer | sets the register to the provided value |
| INC | 0x02 | R[0-10] | increments the register by one |
| DEC | 0x03 | R[0-10] | decrements the register by one |
| LOD | 0x04 | R[0-10], R[0-10] | loads the value pointed to by arg1 into arg2 |
| STR | 0x05 | R[0-10], R[0-10] | stores the value in arg1 into the memory pointed to by arg2 |
| PRI | 0x06 | R[0-10] | prints the value in arg1 as an integer to stdout |
| PRC | 0x07 | R[0-10] | prints the value in arg1 as a character to stdout |
| REQ | 0x08 | integer, R[0-10] | requests space for arg1 integers on the heap, if succesful points arg2 at that memory |
| DEL | 0x09 | R[0-10] | frees the memory used by arg1 |
| CPY | 0x10 | R[0-10], R[0-10] | copys the value of arg1 to arg2 |
| LBL | 0x11 | integer | creates a label with id arg1 |
| JMP | 0x12 | integer | jumps to the label with id arg1 |
| JMZ | 0x13 | R[0-10], integer | jumps to the lable with id arg2 only if arg1's value is zero |
| CAL | 0x14 | integer | adds a frame to the stack, copies over the current register values to the new frame, sets the return address, then jumps to the label |
| RET | 0x15 | n/a | returns to the previous stack frame (removing the current one), and sets the instruction pointer to the return address of the current stack frame |
| RTR | 0x16 | R[0-10] | same as return, but also sets the incoming stack frame's return register to hold the value at arg1 |

The VM has a simple stack/register flow:
- Each stack frame has 10 registers + 1 return register that holds the return value from the previous stacke frame
- The most current stack frame is always the first (LIFO)
- Branching can be done with labels, it is pretty much a GOTO system with the ability for conditional GOTO jumps
- To advance the stack frame with a branch (i.e. get a fresh stack frame), use the CAL operator
    - This will add a new stack frame, and then copy over the register values (not the return register) from the current stack frame and then jump to the corresponding label
    - Since registers are copied, this is the best way to emulate passing "arguments"
- Normal jumps do not advance the stack frame
- Calling a return operator will step back the stack frame (pop out the front), then set the instruction pointer to the new stack frame's return address
    - If you call the return with register operator, it will also copy the current stack frame's value at the specified register into the new stack frame's return register
- Every program is given a set heap (right now its 1024 32 bit integers)
    - It is a bump allocator, with compaction logic in the case when there are no chunks open
    - Memory is still manually managed though, users must claim and free memory