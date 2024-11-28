#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <time.h>

extern void vec_assign_ones(short* x);

void scalar_assign_ones(short* x) {
    for (int i = 0; i < 32; i += 2) {
        x[i] = 1;
    }
}

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

static inline uint64_t rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

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
#define WARMUP_ITERATIONS 0

int main() {
    printf("Total iterations: %d\n", NUM_ITERATIONS);

    // Allocate aligned memory
    short* array = (short*)aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(short));
    if (!array) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    uint64_t start_cycles, end_cycles;
    struct timespec start_time, end_time;
    double time_taken_scalar, time_taken_vector;

    // Test vectorized version
    printf("Testing AVX-512 version:\n");
    reset_array(array);
    
    // Warmup
    // for (int i = 0; i < WARMUP_ITERATIONS; i++) {
    //     vec_assign_ones(array);
    // }

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

    uint64_t vec_total_cycles = end_cycles - start_cycles;
    time_taken_vector = (end_time.tv_sec - start_time.tv_sec) +
                 (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    printf("AVX-512 version:\n");
    printf("  Total cycles: %lu\n", vec_total_cycles);
    printf("  Cycles per iteration: %.2f\n", (double)vec_total_cycles / NUM_ITERATIONS);
    printf("  Total time: %.9f seconds\n", time_taken_vector);
    printf("  Time per iteration: %.2f nanoseconds\n", (time_taken_vector * 1e9) / NUM_ITERATIONS);

    // Test scalar version
    printf("\nTesting scalar version:\n");
    reset_array(array);
    
    // Warmup
    // for (int i = 0; i < WARMUP_ITERATIONS; i++) {
    //     scalar_assign_ones(array);
    // }

    // Start timing
    cpu_serialize();
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    start_cycles = rdtscp();

    // Main test loop
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        reset_array(array);
        scalar_assign_ones(array);
    }

    // End timing
    end_cycles = rdtscp();
    cpu_serialize();
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    if (!verify_array(array)) {
        printf("Scalar version produced incorrect results!\n");
        print_array(array, ARRAY_SIZE);
    }

    uint64_t scalar_total_cycles = end_cycles - start_cycles;
    time_taken_scalar = (end_time.tv_sec - start_time.tv_sec) +
                 (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    printf("Scalar version:\n");
    printf("  Total cycles: %lu\n", scalar_total_cycles);
    printf("  Cycles per iteration: %.2f\n", (double)scalar_total_cycles / NUM_ITERATIONS);
    printf("  Total time: %.9f seconds\n", time_taken_scalar);
    printf("  Time per iteration: %.2f nanoseconds\n", (time_taken_scalar * 1e9) / NUM_ITERATIONS);

    printf("\nSpeedup:\n");
    printf("  Cycles: %.2fx\n", (double)scalar_total_cycles / vec_total_cycles);
    printf("  Time: %.2fx\n", (double)time_taken_scalar / time_taken_vector);

    free(array);
    return 0;
}