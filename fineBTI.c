#include <stdio.h>
#include <stdlib.h>

void memauth(void *ptr, size_t size) {
    asm volatile (
        "mov x0, %0\n"
        "mov x1, %1\n"
        "autia x0, x1\n"
        :
        : "r" (ptr), "r" (size)
        : "x0", "x1"
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