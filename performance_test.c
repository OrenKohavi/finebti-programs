#include "bti_hlp.h"
#include <stdio.h>
#include <time.h>

#define NUM_ITERATIONS 1000000

void __attribute__((noinline)) standard_function() {
    volatile int dont_optimize_me = 0; //To prevent the function from being entirely optimized out
    return;
}

void __attribute__((noinline)) finebti_function() {
    __auth_macro
    volatile int dont_optimize_me = 0; //To prevent the function from being entirely optimized out
    return;
}

int main() {
    volatile void (*func)(); //Declared as volatile to prevent optimization of the function pointer to a direct call

    //Profile the performance without FineBTI (Still protected with standard BTI)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    func = &standard_function;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        //Standard call, written as inline assembly to prevent the compiler from optimizing out the function pointer
        __asm__ volatile("blr %0" :: "r"(func) : "memory");
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long long standard_time_taken = ((end.tv_sec - start.tv_sec) * 1e9) + (end.tv_nsec - start.tv_nsec);

    // Print the time taken in nanoseconds
    printf("Time without FineBTI: %lld nanoseconds\n", standard_time_taken);

    //Profile the performance with FineBTI
    clock_gettime(CLOCK_MONOTONIC, &start);
    func = &finebti_function;
    __pac_macro(func);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        __call_macro_voidreturn(func);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long long finebti_time_taken = ((end.tv_sec - start.tv_sec) * 1e9) + (end.tv_nsec - start.tv_nsec);

    // Print the time taken in nanoseconds
    printf("Time with FineBTI:    %lld nanoseconds\n", finebti_time_taken);

    float performance_ratio = (float)finebti_time_taken / standard_time_taken;
    printf("FineBTI is %.2f times slower than standard BTI\n", performance_ratio);



}
