#include "bti_hlp.h"

inline void *__attribute__((always_inline)) __pac_ptr(void *ptr)
{
    __asm__ __volatile__(
        "pacia %0, %0\n" // Pointer authentication using pacia
        : "+r"(ptr)      // Input/output operand
    );
    return ptr;
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