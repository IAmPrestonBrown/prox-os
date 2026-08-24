#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdlib.h>

#define REGISTER_COUNT 10
#define ALLOCATION_COUNT 10
#define TWO_FLAG 0x80
#define HEAP_SIZE 1024

enum OpCodes {
    SET = 0x01 | TWO_FLAG,  // sets value of register
    INC = 0x02,             // increments a register
    DEC = 0x03,             // decrements a register
    LOD = 0x04,             // load memory at address
    STR = 0x05 | TWO_FLAG,  // store value at address
    PRI = 0x06,             // print integer to stdout
    PRS = 0x07,             // print string to stdout
    REQ = 0x08 | TWO_FLAG,  // request memory from system
    DEL = 0x09,             // free memory at register
    CPY = 0x10 | TWO_FLAG   // copies register values
};

typedef struct MemChunk {
    uint32_t index;
    uint32_t size;
} MemChunk;

typedef struct RegValue {
    uint32_t value;
    uint32_t is_ref;
} RegValue;

typedef struct VirtualMachine {
    RegValue registers[REGISTER_COUNT];
    uint32_t* allocations[ALLOCATION_COUNT];
    
    uint32_t memory[HEAP_SIZE];
    MemChunk* chunks;
    uint32_t chunk_count;
    
    uint32_t alloc_from;
} VirtualMachine;

VirtualMachine create_vm() {
    VirtualMachine vm = {0};
    return vm;
}

void vm_mem_compact(VirtualMachine vm) {
    
}

void destroy_vm(VirtualMachine vm);
int vm_request_mem(VirtualMachine vm, uint32_t size);
int vm_free_mem(VirtualMachine vm, uint32_t addr);
int execute_program(VirtualMachine vm, int* program, int length);

#endif