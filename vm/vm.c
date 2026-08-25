/**
 * ProxOS Virtual Machine impementation
 * 
 * Copyright 2026 Preston Brown
 */

#include "./vm.h"
#include <string.h>

VirtualMachine* create_vm() {
    VirtualMachine* vm = malloc(sizeof(VirtualMachine));
    memset(vm, 0, sizeof(VirtualMachine));
    vm->stack_frames = malloc(sizeof(StackFrame));
    vm->stack_frame_count = 1;

    return vm;
}

void destroy_vm(VirtualMachine* vm) {
    free(vm->stack_frames);
    vm->stack_frames = 0;

    if(vm->label_count) {
        free(vm->labels);
        vm->labels = 0;
        vm->label_count = 0;
    }
    
    // if there are zero chunks, the chunk list is not initiated,
    // so no free is needed
    if(vm->chunk_count) {
        free(vm->chunks);
        vm->chunks = 0;
        vm->chunk_count = 0;        
    }

    free(vm);
}

void vm_mem_compact(VirtualMachine* vm) {
    uint32_t last_chunk_end = 0;
    for(int idx = 0; idx < vm->chunk_count; idx++) {
        // if the chunk doesn't start after the last one finished, move it back
        if(vm->chunks[idx].index > last_chunk_end) {
            // loop through each byte of the chunk and copy it over starting from the end of the
            // previous chunk
            for(int j = vm->chunks[idx].index; j < vm->chunks[idx].index+vm->chunks[idx].size; j++) {
                vm->memory[last_chunk_end+(j-vm->chunks[idx].index)] = vm->memory[j];
            }

            // reassign any register that pointed at that chunk to point to it's new position
            for(int i = 0; i < vm->stack_frame_count; i++) {
                for(int j = 0; j < REGISTER_COUNT; j++) {
                    if(vm->stack_frames[i].registers[j].is_ref && vm->stack_frames[i].registers[j].value == vm->chunks[idx].index) {
                        vm->stack_frames[i].registers[j].value = last_chunk_end;
                    }
                }
            }

            // reassign the chunk index too
            vm->chunks[idx].index = last_chunk_end;
        }
        last_chunk_end = vm->chunks[idx].index+vm->chunks[idx].size;
    }
}

int vm_request_mem(VirtualMachine* vm, uint32_t size) {
    // If the chunk list is not iniatiated, this chunk can be allocated from the start
    if(!vm->chunk_count) {
        // if requested size is greater than the heap, it cannot be allocated
        if(size >= HEAP_SIZE)
            error_out("attempted to allocate more memory than what is available on the heap (size >= HEAP_SIZE)")
        
        // start the chunk list off
        vm->chunks = malloc(sizeof(MemChunk));
        vm->chunk_count = 1;

        // insert the first allocated chunk
        vm->chunks[0].index = 0;
        vm->chunks[0].size = size;

        // allocated index/address will be zero
        return 0;
    }

    // search for the first appearance of open space between chunks that can fit the space we need
    uint32_t last_chunk_end = 0;
    uint32_t open = 0; // stores whether an open chunk was found here

    for(uint32_t chunk_index = 0; chunk_index < vm->chunk_count; chunk_index++) {
        // Optimized expression, essentially checks if the space between the 
        // current chunk and the previous is big enough to fit `size`. The result
        // of that check is stored in open so that the following code outside the loop
        // can access that result.
        if((open = vm->chunks[chunk_index].index-last_chunk_end >= size))
            break;
        
        last_chunk_end = vm->chunks[chunk_index].index+vm->chunks[chunk_index].size;
    }

    // if there were no open chunks and no open space after the last chunk, 
    // no allocation can be made
    if(!open && last_chunk_end+size >= HEAP_SIZE)
        error_out("attempted to allocate more memory than what is available on the heap (potentially non-fatal)")

    // Now insert the new chunk of memory in the chunk list
    // It is important that it is inserted in order, otherwise the open space check will
    // fail (since it assumes the chunks are ordered)

    // create the new chunk's structure first
    MemChunk new_chunk = {.index=last_chunk_end, .size=size};

    // make the space for the list with the new chunk inserted
    MemChunk* fresh_chunks = malloc(sizeof(MemChunk)*(vm->chunk_count+1));
    uint32_t inserted = 0; // marks whether or not the new chunk has been inserted yet

    for(uint32_t i = 0; i < vm->chunk_count; i++) {
        if(!inserted && new_chunk.index < vm->chunks[i].index) {
            fresh_chunks[i] = new_chunk;
            inserted = 1;
        }
        fresh_chunks[i+inserted] = vm->chunks[i];
    }
    if(!inserted) fresh_chunks[vm->chunk_count] = new_chunk;

    // point to the new chunk list
    free(vm->chunks);
    vm->chunks = fresh_chunks;
    vm->chunk_count += 1;

    return new_chunk.index;
}

int vm_free_mem(VirtualMachine* vm, uint32_t addr) {
    uint32_t found = 0;
    for(uint32_t i = 0; i < vm->chunk_count; i++)
        if((found = vm->chunks[i].index == addr)) break;
    if(!found)
        error_out("attempted to free unallocated memory")
    
    // find the chunk to free and remove it from the list (by skipping)
    MemChunk* fresh_chunks = malloc(sizeof(MemChunk)*(vm->chunk_count-1));
    uint32_t skip = 0;
    
    for(int i = 0; i < vm->chunk_count; i++) {
        if(vm->chunks[i].index == addr) {
            skip = 1;
            continue;
        }
        fresh_chunks[i-skip] = vm->chunks[i];
    }
    
    // replace the old list with the new
    free(vm->chunks);
    vm->chunks = fresh_chunks;
    vm->chunk_count -= 1;
    
    return 0;
}

#define get_label(uid, vm, lid) int label_pos_##uid = -1;\
for(int i = 0; i < vm->label_count; i++) {\
    if(vm->labels[i].id == lid) {\
        label_pos_##uid = vm->labels[i].pos;\
        break;\
    }\
}

int execute_program(VirtualMachine* vm, int* program, int length) {
    // header format
    // first 4 ints: 80 82 79 88
    // next int: label count

    // collect all the labels first (pass 1)
    vm->labels = malloc(sizeof(Label)*program[4]);
    vm->label_count = program[4];

    int label_idx = 0;
    for(int i = 5; i < length; i++) {
        int opcode = program[i];
        if(++i >= length)
            break;

        if(opcode == LBL) {
            vm->labels[label_idx++] = (Label){.id = program[i], .pos = i+1};
        }

        i += !!(opcode&TWO_FLAG);
    }

    // execute the program (pass 2)
    for(int i = 5; i < length; i++) {
        // opcode will always be the first item
        int opcode = program[i];
        
        // Optimized expression to get both the arguments regardless whether or not both are expected
        // There could be a case where we pull two arguments ahead for a one argument operator at the end 
        // of the program, so the index is escaped with an overflow protection (index set to 0 on overflow, 
        // never accesses out of bounds)
        int args[2] = {program[((i+1)<length)*(i+1)], program[((i+2)<length)*(i+2)]};
        i += 1+!!(opcode&TWO_FLAG);

        // OpCode specific handling
        switch(opcode) {
            case NOP:
            case LBL: {
                continue;
            }
            
            case SET: {
                vm->stack_frames[0].registers[args[0]].value = args[1];
                vm->stack_frames[0].registers[args[0]].is_ref = 0;
                continue;
            }
            
            case INC: {
                vm->stack_frames[0].registers[args[0]].value++;
                continue;
            }
            
            case DEC: {
                vm->stack_frames[0].registers[args[0]].value--;
                continue;
            }

            case LOD: {
                vm->stack_frames[0].registers[args[1]].value = vm->memory[vm->stack_frames[0].registers[args[0]].value];
                vm->stack_frames[0].registers[args[1]].is_ref = 0;
                continue;
            }

            case STR: {
                vm->memory[vm->stack_frames[0].registers[args[0]].value] = args[1];
                continue;
            }

            case REQ: {
                vm->stack_frames[0].registers[args[1]].value = vm_request_mem(vm, vm->stack_frames[0].registers[args[0]].value);
                vm->stack_frames[0].registers[args[1]].is_ref = 1;

                // if the allocation failed, try compacting and allocating again
                if(vm->stack_frames[0].registers[args[1]].value == -1) {
                    vm_mem_compact(vm);
                    vm->stack_frames[0].registers[args[1]].value = vm_request_mem(vm, vm->stack_frames[0].registers[args[0]].value);
                    vm->stack_frames[0].registers[args[1]].is_ref = 1;

                    // if it still failed, than the heap just doesn't have enough free space
                    if(vm->stack_frames[0].registers[args[1]].value == -1)
                        error_out("attempted to allocate more memory than what is available on the heap (fatal, post-compaction)")
                }

                continue;
            }

            case DEL: {
                if(vm_free_mem(vm, vm->stack_frames[0].registers[args[0]].value) == -1)
                    error_out("free operator failed")
                vm->stack_frames[0].registers[args[0]].value = 0;
                vm->stack_frames[0].registers[args[0]].is_ref = 0;
                continue;
            }

            case CPY: {
                vm->stack_frames[0].registers[args[1]] = vm->stack_frames[0].registers[args[0]];
                continue;
            }

            case PRI: {
                fprintf(stdout, "%i", vm->stack_frames[0].registers[args[0]].value);
                fflush(stdout);
                continue;
            }

            case PRC: {
                fprintf(stdout, "%c", vm->stack_frames[0].registers[args[0]].value);
                fflush(stdout);
                continue;
            }

            case JMP: {
                get_label(1, vm, args[0]);
                if(label_pos_1 == -1)
                    error_out("attempted to jump to non-existing label (unconditional)")
                i = label_pos_1-1;
                continue;
            }

            case JMZ: {
                get_label(2, vm, args[1]);
                if(label_pos_2 == -1)
                    error_out("attempted to jump to non-existing label (conditional)")
                
                if(vm->stack_frames[0].registers[args[0]].value == 0)
                    i = label_pos_2-1;

                continue;
            }

            case CAL: {
                // expand the stack frames
                StackFrame* fresh_stack_frames_call = malloc(sizeof(StackFrame)*++vm->stack_frame_count);
                memcpy(fresh_stack_frames_call+1, vm->stack_frames, sizeof(StackFrame)*(vm->stack_frame_count-1));
                free(vm->stack_frames);
                
                // insert the new one at the start
                vm->stack_frames = fresh_stack_frames_call;             
                vm->stack_frames[0].return_address = i+1;
                vm->stack_frames[0].registers[REGISTER_COUNT] = (RegValue){.value=0, .is_ref=0};
                
                // copy over register values from told stack frame to start the new one
                memcpy(vm->stack_frames[0].registers, vm->stack_frames[1].registers, sizeof(RegValue)*REGISTER_COUNT);

                // Jump to the specified label
                get_label(3, vm, args[0]);
                if(label_pos_3 == -1)
                    error_out("attempted to jump to non-existing label (call operation)")
                i = label_pos_3-1;

                continue;
            }

            case RET: {
                if(vm->stack_frame_count == 1)
                    error_out("attempted to return from stack start");
                
                i = vm->stack_frames[0].return_address-1;
                StackFrame* fresh_stack_frames_ret = malloc(sizeof(StackFrame)*--vm->stack_frame_count);
                memcpy(fresh_stack_frames_ret, vm->stack_frames+1, sizeof(StackFrame)*vm->stack_frame_count);
                free(vm->stack_frames);
                vm->stack_frames = fresh_stack_frames_ret;
                continue;
            }

            case RTR: {
                if(vm->stack_frame_count == 1)
                    error_out("attempted to return from stack start");
                
                RegValue return_register_value = vm->stack_frames[0].registers[args[0]];
                i = vm->stack_frames[0].return_address-1;
                StackFrame* fresh_stack_frames = malloc(sizeof(StackFrame)*--vm->stack_frame_count);
                memcpy(fresh_stack_frames, vm->stack_frames+1, sizeof(StackFrame)*vm->stack_frame_count);
                free(vm->stack_frames);
                vm->stack_frames = fresh_stack_frames;
                vm->stack_frames[0].registers[REGISTER_COUNT] = return_register_value;
                continue;
            }
        }
    }

    return 0;
}