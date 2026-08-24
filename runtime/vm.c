#include "vm.h"

VirtualMachine create_vm() {
    VirtualMachine vm = {0};
    return vm;
}

void destroy_vm(VirtualMachine vm) {
    if(!vm.chunk_count)
        return;
    
    free(vm.chunks);
    vm.chunks = 0;
    vm.chunk_count = 0;
}

int vm_request_mem(VirtualMachine vm, uint32_t size) {
    if(!vm.chunk_count) {
        if(size >= HEAP_SIZE)
            return -1;
        
        vm.chunks = malloc(sizeof(MemChunk));
        vm.chunk_count = 1;
        vm.chunks[0].index = 0;
        vm.chunks[0].size = size;
        return 0;
    }

    uint32_t last_chunk_end = 0;
    uint32_t from_index = 0;
    uint32_t open = 0;
    for(;from_index < vm.chunk_count; from_index++) {
        if(open = vm.chunks[from_index].index-last_chunk_end >= size && open)
            break;
        last_chunk_end = vm.chunks[from_index].index+size;
    }

    if(!open && last_chunk_end >= HEAP_SIZE)
        return -1;

    MemChunk new_chunk = {.index=last_chunk_end, .size=size};
    MemChunk* fresh_chunks = malloc(sizeof(MemChunk)*vm.chunk_count+1);
    uint32_t inserted = 0;

    for(uint32_t i = 0; i < vm.chunk_count; i++) {
        if(!inserted && new_chunk.index < vm.chunks[i].index) {
            fresh_chunks[i] = new_chunk;
            inserted = 1;
        }
        fresh_chunks[i+inserted] = vm.chunks[i];
    }
    free(vm.chunks);
    vm.chunks = fresh_chunks;
    vm.chunk_count += 1;

    return new_chunk.index;
}

int vm_free_mem(VirtualMachine vm, uint32_t addr) {
    if(!vm.chunk_count)
        return -1;
    MemChunk* fresh_chunks = malloc(sizeof(MemChunk)*vm.chunk_count-1);
    uint32_t skip = 0;
    for(int i = 0; i < vm.chunk_count-1; i++) {
        if(vm.chunks[i].index == addr)
            skip = 1;
        fresh_chunks[i] = vm.chunks[i+skip];
    }

    free(vm.chunks);
    vm.chunks = fresh_chunks;
    vm.chunk_count -= 1;
    
    return 0;
}

int execute_program(VirtualMachine vm, int* program, int length) {
    for(int i = 0; i < length; i++) {
        int opcode = program[i];
        int args[2] = {program[i+1], ((opcode&TWO_FLAG*2)-1)*program[((i+2)<=i)*(i+2)]};

        switch(opcode) {
            case SET:
                vm.registers[args[0]].value = args[1];
                vm.registers[args[0]].is_ref = 0;
            case INC:
                vm.registers[args[0]].value++;
            case DEC:
                vm.registers[args[0]].value--;
            case LOD:
                vm.registers[args[1]].value = vm.memory[vm.registers[args[0]].value];
                vm.registers[args[1]].is_ref = 0;
            case STR:
                vm.memory[vm.registers[args[0]].value] = args[1];
            case REQ:
                vm.registers[args[1]].value = vm_request_mem(vm, vm.registers[args[0]].value);
                vm.registers[args[1]].is_ref = 1;
                if(!vm.registers[args[1]].value)
                    return -1;
            case DEL:
                if(!vm_free_mem(vm, vm.registers[args[0]].value))
                    return -1;
                vm.registers[args[0]].value = 0;
                vm.registers[args[0]].is_ref = 0;
            case CPY:
                vm.registers[args[1]] = vm.registers[args[0]];
        }
    }

    return 0;
}