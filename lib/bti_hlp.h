#ifndef BTI_HLP_H
#define BTI_HLP_H

#include <stdint.h>
#include <stdlib.h>

//Function prototypes for public functions

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


#define __pac_macro(ptr) ({ \
    __asm__ __volatile__( \
        "pacia %0, %0\n" \
        : "+r"(ptr) \
    ); \
})


/*
Since compiler support is not here yet, I can't force the system to use the `blraa` instruction
As a result, this macro needs to be invoked for every call.
Future compiler support could replace this whole thing with a 'blraa' instruction.
*/
#define __call_macro(func, ...) ({                   \
    typeof(func(__VA_ARGS__)) __result;              \
    typedef typeof(func) func_ptr_type;              \
    func_ptr_type temp_func = func;                  \
    temp_func = (func_ptr_type)((uintptr_t)temp_func & 0xFFFFFFFFFFFF);          \
    __asm__ volatile (                               \
        "mov x8, %0\n\t"                             \
        :                                            \
        : "r"(func)                             \
        : "x8"                                       \
    );                                               \
    __result = temp_func(__VA_ARGS__);               \
    __result;                                        \
})



#define __auth_macro {                              \
    __asm__ volatile (                              \
        "mov x9, x8\n"                              \
        "and x9, x9, #0xffffffffffff\n"             \
        "autia x8, x9\n"                            \
        :                                           \
        :                                           \
        : "x9"                                      \
    );                                              \
}

#endif // BTI_HLP_H