#include "bti_hlp.h"

// Calls a pointer that has been authenticated with PACIA
inline void __attribute__((always_inline)) __call(void *ptr, ...)
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

inline void *__attribute__((always_inline)) __pac_ptr(void *ptr)
{
    __asm__ __volatile__(
        "pacia %0, %0\n" // Pointer authentication using pacia
        : "+r"(ptr)      // Input/output operand
    );
    return ptr;
}

inline void __attribute__((always_inline)) __aut_ptr(void *ptr)
{
    __asm__ __volatile__(
        "autia %0, %0\n" // Address authentication using autia
        :
        : "r"(ptr) // Input operand
    );
}

inline void __attribute__((always_inline)) __aut_ptr_mask_x8()
{
    __asm__ __volatile__(
        "mov x9, x8\n"
        "and x9, x9, #0xffffffffffff\n"
        "autia x8, x9\n"
    );
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