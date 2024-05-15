#ifndef BTI_HLP_H
#define BTI_HLP_H

#include <stdint.h>
#include <stdlib.h>

//Function prototypes for public functions

void memauth(void *beginning, size_t len);

//Function prototypes for private functions (i.e. those that would ideally be replaced by compiler magic later)

void *__pac_ptr(void *ptr);
void __call_in_x8(void *ptr, ...);
void __aut_ptr_mask_x8();

// Less used:

//void __call(void *ptr);
void __aut_ptr(void *ptr);


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


//Macro for making authenticating pointers easier
#define __aut_ptr_macro(reg) {                    \
    void* __aut_ptr_value;                        \
    asm volatile (                                \
        "mov %0, " reg "\n"                       \
        : "=r" (__aut_ptr_value)  /* output */    \
        : /* No inputs */                         \
        : /* Nothing clobbered */                 \
    );                                            \
    __aut_ptr(__aut_ptr_value);                   \
    /*__asm__ volatile ("" __func__ "_nobti:") /* label for the version directly called */     \
}

#endif // BTI_HLP_H