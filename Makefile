CC=clang
CFLAGS=-O3 -Wall -Wextra
AVX_FLAGS=-mavx512f
NASM=nasm
NASMFLAGS=-f elf64

all: benchmark_scalar benchmark_avx512

benchmark_scalar: bench/benchmark_scalar.c
	$(CC) $(CFLAGS) $^ -o bench/$@

# First assemble the NASM file, then compile and link with clang
bench/vec_impl.o: bench/vec_impl.nasm
	$(NASM) $(NASMFLAGS) $< -o $@

benchmark_avx512: bench/benchmark_avx512.c bench/vec_impl.o
	$(CC) $(CFLAGS) $(AVX_FLAGS) $^ -o bench/$@

clean:
	rm -f bench/benchmark_scalar bench/benchmark_avx512 bench/bench/*.o

run_scalar: benchmark_scalar
	./bench/benchmark_scalar

run_avx512: benchmark_avx512
	./bench/benchmark_avx512

.PHONY: all clean run_scalar run_avx512 