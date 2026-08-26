/**
 * ProxOS Virtual Machine test
 *
 * Copyright 2026 Preston Brown
 */

#include <prox/vm/vm.h>

int main(void) {
    uint32_t prx_test_program[] = {80, 82, 79, 88, 8, 0, 0, 129, 0, 3, 136, 0, 1, 144, 1, 2, 133, 1, 72, 2, 1, 133, 1, 105, 2, 1, 133, 1, 0, 144, 2, 1, 20, 6, 132, 1, 3, 7, 3, 129, 4, 5, 17, 1, 6, 4, 3, 4, 147, 4, 2, 18, 1, 17, 2, 129, 5, 4, 136, 5, 6, 20, 3, 129, 7, 41, 20, 4, 6, 10, 20, 3, 9, 6, 9, 2, 129, 0, 300, 136, 0, 1, 136, 0, 2, 136, 0, 3, 9, 1, 129, 0, 100, 136, 0, 4, 9, 2, 129, 0, 550, 136, 0, 5, 6, 3, 20, 3, 6, 5, 20, 3, 18, 5, 17, 3, 129, 8, 10, 7, 8, 21, 0, 17, 4, 2, 7, 22, 7, 17, 6, 17, 7, 132, 1, 9, 147, 9, 8, 7, 9, 2, 1, 18, 7, 17, 8, 21, 0, 17, 5, 0, 0};
    uint32_t prx_oversized_alloc_program[] = {80, 82, 79, 88, 0, 129, 0, 2000, 136, 0, 1};
    uint32_t prx_invalid_free_program[] = {80, 82, 79, 88, 0, 129, 0, 3, 136, 0, 1, 129, 2, 999, 9, 2};
    uint32_t prx_missing_jump_label_program[] = {80, 82, 79, 88, 0, 18, 99};
    uint32_t prx_missing_branch_label_program[] = {80, 82, 79, 88, 0, 147, 0, 99};
    uint32_t prx_missing_call_label_program[] = {80, 82, 79, 88, 0, 20, 99};
    uint32_t prx_stack_underflow_program[] = {80, 82, 79, 88, 0, 21, 0};
    uint32_t prx_stack_underflow_return_program[] = {80, 82, 79, 88, 0, 22, 0};
    uint32_t prx_truncated_program[] = {80, 82, 79, 88, 0, 0};

#ifndef VM_DEBUG
#warning "You are building outside of debug mode. To get full debug info, build with -DVM_DEBUG"
#endif

    VirtualMachine* vm = create_vm();
    printf("--- Running test ---\nExpected output:\nHiH54321\n42\nattempted to allocate more memory than what is available on the heap (potentially non-fatal)\n100\n400\n0\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_test_program, 153));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to allocate more memory than what is available on the heap (size >= HEAP_SIZE)\nattempted to allocate more memory than what is available on the heap (size >= HEAP_SIZE)\nattempted to allocate more memory than what is available on the heap (fatal, post-compaction)\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_oversized_alloc_program, 11));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to free unallocated memory\nfree operator failed\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_invalid_free_program, 16));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to jump to non-existing label (unconditional)\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_missing_jump_label_program, 7));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to jump to non-existing label (conditional)\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_missing_branch_label_program, 8));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to jump to non-existing label (call operation)\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_missing_call_label_program, 7));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to return from stack start\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_stack_underflow_program, 7));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\nattempted to return from stack start\n-1\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_stack_underflow_return_program, 7));
    printf("--------------------\n");
    destroy_vm(vm);

    vm = create_vm();
    printf("--- Running test ---\nExpected output:\n0\n--------------------\n");
    printf("%i\n", execute_program(vm, prx_truncated_program, 6));
    printf("--------------------\n");
    destroy_vm(vm);
}
