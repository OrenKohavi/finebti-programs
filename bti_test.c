#include <stdio.h>
#include <stdlib.h>


int target_function() {
    __asm__ volatile (
        "mov x0, #5\n"
        "ret\n"
        "mov x0, #123\n\t"
        "ret\n\t"
    );
}

int main() {
    int (*func_ptr)() = target_function;

    char *misaligned_ptr = (char *)func_ptr + (3 * 4); //Skip 3 instructions (BTI instruction, Mov, and Ret)
    int (*misaligned_func_ptr)() = (int (*)())misaligned_ptr;

    //Sanity check that the real pointer works
    int result = 0;
    result = func_ptr();
    if (result != 5){
        perror("Something is fundamentally wrong\n");
        exit(1);
    }
    printf("Attempting to run a gadget.. BTI should stop this!\n");
    fflush(stdout);
    result = misaligned_func_ptr();
    printf("[!] BTI Did not prevent calling a misalinged function!\n");
    if (result == 123){
        printf("[!] Gadget executed succesfully: BTI is not working\n");
        exit(0);
    } else {
        printf("[-] Gadget executed wrong, troubleshoot this.\n");
    }

    return 0;
}
