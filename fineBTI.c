#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Calls a pointer that has been authenticated with PACIA
static inline void __attribute__((always_inline)) call(void *ptr)
{
    /* blr doesn't play nice with PAC-ed pointers, so we need to use blraa
     * But using blraa would mean that we need to get the modifier out of the pointer
     * ^ which is more overhead and is done on the callee side anyways.
     * The compromise here: mask out the modifier and use blr in a copy of the pointer
     */

    asm volatile(
        "mov x9, %0\n"
        "mov x10, #0xffffffffffff\n"
        "and x9, x9, x10\n"
        "blr x9\n"
        :
        : "r"(ptr)
        :); // This is basically a blr instruction for PAC-ed pointers.
}

void memauth(void *beginning, size_t len)
{
    uintptr_t *ptr = (uintptr_t *)beginning;
    uintptr_t *end = ptr + len;

    asm volatile(
        "1:\n"                 // Label 1 for the loop
        "cmp %0, %1\n"         // Compare current pointer with end pointer
        "b.ge 2f\n"            // If current pointer >= end pointer, branch to label 2
        "ldr x2, [%0]\n"       // Load the value from the current pointer
        "pacia x2, x2\n"       // Authenticate the value with itself as the modifier
        "str x2, [%0]\n"       // Store the authenticated value back to the current pointer
        "add %0, %0, #8\n"     // Move to the next pointer (8 bytes ahead)
        "b 1b\n"               // Branch back to label 1
        "2:\n"                 // Label 2 for the end
        : "+r"(ptr)            // Output operand: update ptr
        : "r"(end)             // Input operand: end pointer
        : "x2", "cc", "memory" // Clobbered registers and flags
    );
}

int protected() {
    asm volatile (
        "mov x9, x8\n"
        "mov x10, #0xffffffffffff\n"
        "and x9, x9, x10\n"
        "autia x8, x9\n"
        // Function stuff goes here
        "mov x0, #234\n"
        "ret\n"
    );
}

int unprotected() {
    asm volatile (
        // Function stuff goes here
        "mov x0, #567\n"
        "ret\n"
    );
}


int main(){

    int (*protected_ptr)() = protected;
    int (*protected_ptr_unsigned)() = protected; 
    asm volatile(
        "pacia %1, %0\n"
        : "=r" (protected_ptr)
        : "r" (protected_ptr)
        :
    );

    int (*unprotected_ptr)() = unprotected;

    int result = 0;
    result = unprotected_ptr();
    if (result != 567){
        printf("Something is fundamentally wrong\n");
        exit(1);
    } else {
        printf("Unprotected function works as gadget\n");
    }

    asm volatile ("mov x8, %1\n" "blraa x8, %2\n" "mov %0, x0\n" : "=r" (result) : "r" (protected_ptr), "r" (protected_ptr_unsigned) : "x8");
    if (result != 234){
        printf("Something is fundamentally wrong (result: %d)\n", result);
        exit(1);
    } else {
        printf("Protected function works when signed\n");
    }

    result = protected_ptr_unsigned();
    if (result != 234){
        printf("Something is fundamentally wrong\n");
        exit(1);
    } else {
        printf("protected function works when unsigned, very bad!\n");
    }

}