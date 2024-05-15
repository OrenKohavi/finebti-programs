#include "finebti.h"
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

volatile int err_expected = 0;
static jmp_buf jump_buffer;

void sigill_handler(int sig) {
    if (err_expected == 0) {
        printf("[-] Caught SIGILL when it was not expected. Inconclusive results\n");
        exit(1);
    }
    longjmp(jump_buffer, 1);
}

int add(int a, int b){
    __auth_macro
    return a + b;
}

int main(){
    if (signal(SIGILL, sigill_handler) == SIG_ERR) {
        perror("Failed to set signal handler\n");
        exit(1);
    }


    printf("[+] Testing FineBTI under normal conditions\n");
    int (*add_ptr)(int, int) = add;
    __pac_macro(add_ptr);

    int result = __call_macro(add_ptr, 5, 6);
    if (result == 11){
        printf("[+] FineBTI works as expected\n");
    } else {
        printf("[-] FineBTI failed\n");
        exit(1);
    }

    printf("---\n");
    printf("[+] Testing FineBTI without a PAC-ed pointer\n");
    add_ptr = &add;
    //No __pac_macro here

    if (setjmp(jump_buffer) == 0){
        err_expected = 1;
        result = __call_macro(add_ptr, 7, 8);
        err_expected = 0;
        //If we get here, BTI did not work
        printf("[-] FineBTI failed, able to call a function through an un-encrypted pointer\n");
        exit(1);
    } else {
        printf("[+] Caught SIGILL, FineBTI works as expected\n");
    }

    exit(0);
}