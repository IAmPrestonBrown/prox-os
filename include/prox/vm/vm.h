/**
 * ProxOS Virtual Machine
 *
 * Executes from a custom bytecode format as a bytecode interpreter.
 * Function similarly to MachO or Elf formats in terms of its function and
 * place in the OS.
 *
 * Copyright 2026 Preston Brown
 */

#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VM_DEBUG
#define error_out(msg)                \
    do {                              \
        fprintf(stderr, "%s\n", msg); \
        fflush(stderr);               \
        return -1;                    \
    } while(0);
#else
#define error_out(msg) \
    do {               \
        return -1;     \
    } while(0);
#endif

// memory/register configurations
#define REGISTER_COUNT 10 // set active register count
#define HEAP_SIZE 1024

// flag used to determine if an operator takes two arguments
#define TWO_FLAG 0x80

// All PRX Bytecode Operators
enum OpCodes {
    NOP = 0x00,            // empty operator
    SET = 0x01 | TWO_FLAG, // sets value of register
    INC = 0x02,            // increments a register
    DEC = 0x03,            // decrements a register
    LOD = 0x04 | TWO_FLAG, // load memory at address
    STR = 0x05 | TWO_FLAG, // store into the address a value
    PRI = 0x06,            // print integer to stdout
    PRC = 0x07,            // print char to stdout
    REQ = 0x08 | TWO_FLAG, // request memory from system
    DEL = 0x09,            // free memory at register
    CPY = 0x10 | TWO_FLAG, // copies register values
    LBL = 0x11,            // defines a label point
    JMP = 0x12,            // jumps to a label unconditionally
    JMZ = 0x13 | TWO_FLAG, // if register is zero, jumps to a label
    CAL = 0x14,            // grows the stack frame, sets the return address, and then jumps to the label
    RET = 0x15,            // returns back to the previous stack frame
    RTR = 0x16,            // returns back to the previous stack frame while setting the return register
};

// Structure to define a chunk of allocated memory
typedef struct MemChunk {
    uint32_t index;
    uint32_t size;
} MemChunk;

// Structure to define a register's value and if that value is an allocation
typedef struct RegValue {
    uint32_t value;
    uint32_t is_ref;
} RegValue;

// Structure to define a label and where it starts in the bytecode
typedef struct Label {
    uint32_t id;
    uint32_t pos;
} Label;

typedef struct StackFrame {
    RegValue registers[REGISTER_COUNT + 1];
    uint32_t return_address;
} StackFrame;

// Main execution structure
typedef struct VirtualMachine {
    StackFrame* stack_frames;
    uint32_t stack_frame_count;

    // dynamically growing list to show what chunks of the heap are taken
    MemChunk* chunks;
    uint32_t chunk_count;

    Label* labels;
    uint32_t label_count;

    uint32_t memory[HEAP_SIZE]; // the "heap"
} VirtualMachine;

// Creates a new, empty vm
VirtualMachine* create_vm(void);

// Frees any allocations on the VM
void destroy_vm(VirtualMachine* vm);

// Compacts/defrags all the memory chunks in the VM
// Detects any references in registers and reassigns them
// to new points
void vm_mem_compact(VirtualMachine* vm);

// Attempts to get this chunk of memory from the VM heap
// Returns -1 if there was no open chunks sufficient for
// the requested size.
int vm_request_mem(VirtualMachine* vm, uint32_t size);

// Attempts to free the memory allocated at the address
// Returns -1 if the provided address is not allocated memory
int vm_free_mem(VirtualMachine* vm, uint32_t addr);

// Executes an entire PRX bytecode program from start to finish
// Returns -1 on first occurence of error
int execute_program(VirtualMachine* vm, uint32_t* program, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
