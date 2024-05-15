#ifndef BTI_HLP_H
#define BTI_HLP_H

#include <stdint.h>
#include <stdlib.h>

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


void memauth(void **beginning, size_t len)
{
    //To ensure that whole-function reuse isn't possible with memauth, it should NEVER be instrumented with BTI instructions
    //Allowing this function to be indirectly called would allow for a complete bypass of FineBTI
    //It shou
    for(int i = 0; i < len; i ++)
    {
        //Load pointer
        void *ptr = beginning[i];
        //pac pointer
        __pac_macro(ptr);
        //write back
        beginning[i] = 0;
    }
}


#endif // BTI_HLP_H