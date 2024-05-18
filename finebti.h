#ifndef FINEBTI_H
#define FINEBTI_H

#include <stdint.h>
#include <stdlib.h>

#define __pac_macro(ptr) ({ \
    if (ptr != NULL){       \
        __asm__ __volatile__( \
            "pacia %0, %0\n" \
            : "+r"(ptr) \
        ); \
    }  \
})

#define __pac_macro_memory(ptr, mod) ({ \
    __asm__ __volatile__( \
        "pacia %0, %1\n" \
        : "+r"(ptr) \
        : "r"(mod) \
    ); \
})

#define __aut_macro_memory(ptr, mod) ({ \
    __asm__ __volatile__( \
        "autia %0, %1\n" \
        "and %0, %0, #0xffffffffffff\n" \
        "pacia %0, %0\n" \
        : "+r"(ptr) \
        : "r"(mod) \
    ); \
})

/*
Since compiler support is not here yet, I can't force the system to use the `blraa` instruction
As a result, this macro needs to be invoked for every call.
*/
#define __call_macro(func, ...) ({                   \
    typeof(func(__VA_ARGS__)) __result;              \
    typedef typeof(func) func_ptr_type;              \
    func_ptr_type temp_func = func;                  \
    temp_func = (func_ptr_type)((uintptr_t)temp_func & 0xFFFFFFFFFFFF);  \
    __asm__ volatile (                               \
        "mov x8, %0\n"                               \
        "mov x9, %1\n"                               \
        :                                            \
        : "r"(func), "r"(temp_func)                  \
        : "x8", "x9"                                 \
    );                                               \
    __result = temp_func(__VA_ARGS__);               \
    __result;                                        \
})

#define __call_macro_voidreturn(func, ...) ({        \
    typedef typeof(func) func_ptr_type;              \
    func_ptr_type temp_func = func;                  \
    temp_func = (func_ptr_type)((uintptr_t)temp_func & 0xFFFFFFFFFFFF);  \
    __asm__ volatile (                               \
        "mov x8, %0\n"                               \
        "mov x9, %1\n"                               \
        :                                            \
        : "r"(func), "r"(temp_func)                  \
        : "x8", "x9"                                 \
    );                                               \
    temp_func(__VA_ARGS__);                          \
})


#define __call_macro_voidreturn_noargs(func) ({       \
    __asm__ volatile (                                \
        "mov x8, %0\n"                                \
        "and x9, x8, 0xffffffffffff\n"                \
        "blr x8, x9\n"                              \
        :                                             \
        : "r"(func)                                   \
        : "x8", "x9", "memory"                        \
    );                                                \
})


/*
Makes the assumption that the function pointer that was used to call this function is in x8
Future work on compiler passes could ensure that the compiler respects this assumption
^ Currently, this assumption is enforced by using __call_macro to call functions.
*/
#define __auth_macro {                              \
    __asm__ volatile (                              \
        "and x9, x8, #0xffffffffffff\n"             \
        "autia x8, x9\n"                            \
        :                                           \
        :                                           \
        : "x9"                                      \
    );                                              \
}

#define __pac_macro_mem

#endif // FINEBTI_H