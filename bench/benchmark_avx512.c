#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <time.h>

extern void vec_assign_ones(short* x);

int verify_array(short* arr) {
    for (int i = 0; i < 32; i++) {
        if ((i % 2 == 0 && arr[i] != 1) || 
            (i % 2 == 1 && arr[i] != 0)) {
            return 0;
        }
    }
    return 1;
}

void reset_array(short* arr) {
    for (int i = 0; i < 32; i++) {
        arr[i] = 0;
    }
}

void print_array(short* arr, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    printf("\n");
}

// static inline uint64_t rdtsc(void) {
//     unsigned int lo, hi;
//     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
//     return ((uint64_t)hi << 32) | lo;
// }

static inline uint64_t rdtscp(void) {
    unsigned int lo, hi, aux;
    __asm__ __volatile__ ("rdtscp" : "=a" (lo), "=d" (hi), "=c" (aux));
    return ((uint64_t)hi << 32) | lo;
}

static inline void cpu_serialize(void) {
    _mm_lfence();
}

#define NUM_ITERATIONS 1000
#define ARRAY_SIZE 32
#define ALIGNMENT 64

int main() {
    printf("AVX-512 Benchmark\n");
    printf("Total iterations: %d\n", NUM_ITERATIONS);

    // Allocate aligned memory
    short* array = (short*)aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(short));
    if (!array) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    double time_taken;

    // Test vectorized version
    printf("\nTesting AVX-512 version:\n");
    reset_array(array);

    // Start timing
    cpu_serialize();
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtscp();

    // Main test loop
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        reset_array(array);
        vec_assign_ones(array);
    }

    // End timing
    end_cycles = rdtscp();
    cpu_serialize();
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    if (!verify_array(array)) {
        printf("AVX-512 version produced incorrect results!\n");
        print_array(array, ARRAY_SIZE);
    }

    uint64_t total_cycles = end_cycles - start_cycles;
    time_taken = (end_time.tv_sec - start_time.tv_sec) +
                 (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    printf("Results:\n");
    printf("  Total cycles: %lu\n", total_cycles);
    printf("  Cycles per iteration: %.2f\n", (double)total_cycles / NUM_ITERATIONS);
    printf("  Total time: %.9f seconds\n", time_taken);
    printf("  Time per iteration: %.2f nanoseconds\n", (time_taken * 1e9) / NUM_ITERATIONS);

    free(array);
    return 0;
} 