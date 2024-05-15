#include "bti_hlp.h"
#include <stdio.h>

int add(int a, int b){
    __auth_macro
    return a + b;
}

int main(){
    printf("Calling add\n");
    int (*add_ptr)(int, int) = add;
    add_ptr = __pac_ptr(add_ptr);

    int result = __call_macro(add_ptr, 5, 6);
    printf("Result: 5+6=%d\n", result);

    exit(0);
}