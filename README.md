# Linear Algebra Library in C and x86-64 Assembly

A high-performance linear algebra library implemented from scratch in C and x86-64 MASM assembly. The project focuses on low-level optimization techniques, SIMD instructions, and performance benchmarking.

The library currently implements dot product and matrix multiplication, with separate C and assembly implementations to compare performance.

## Features

- Dot product implementation in C and x86-64 assembly.
- Matrix multiplication implementation in C and x86-64 assembly.
- AVX2 SIMD vector operations.
- Fused Multiply-Add (FMA) instructions.
- 4×4 register blocking for matrix multiplication.
- Vectorized processing with scalar handling for remaining elements.
- Microsoft x64 calling convention.
- Benchmarking and numerical correctness tests.

## Performance

The assembly implementations are benchmarked against the C implementations using identical input data.

### Benchmark Results

Tested on an Intel Core i7-12700H processor.

| Operation | Matrix / Vector Size | C | Assembly | Speedup |
|---|---:|---:|---:|---:|
| Dot Product | 262,144 elements | 0.122147 ms | 0.066040 ms | 1.85× |
| Matrix Multiplication | 512×512 | 272.761755 ms | 21.789280 ms | **12.52×** |

The benchmark uses:
- Optimized C compilation with `/O2`.
- Warm-up iterations before measurement.
- Nine timed samples per implementation.
- Median execution time to reduce timing variability.
- Numerical comparison between C and assembly results.

These results represent local measurements and may vary depending on CPU frequency, system load, compiler settings, and hardware.

## Project Structure

```text
LinearAlgebraFromScratch/
│
├── src/
│   ├── LinearAlgebra.h
│   ├── LinearAlgebraNaive.c
│   ├── LinearAlgebra.asm
│   ├── Tests.c
│   └── Benchmark.c
│
├── README.md
├── .gitignore
```

## Requirements

- Windows x64
- Visual Studio 2022 or Visual Studio Build Tools
- Microsoft C/C++ compiler (`cl.exe`)
- Microsoft Macro Assembler x64 (`ml64.exe`)
- CPU supporting AVX2 and FMA

The assembly implementation uses AVX2 and FMA instructions. Running it on processors without these features may cause an illegal-instruction exception.

## Building

Open the **x64 Native Tools Command Prompt for Visual Studio 2022** and navigate to the project directory.

### Assemble the MASM source

```bat
ml64 /c /Fo src\LinearAlgebra.obj src\LinearAlgebra.asm
```

### Build and run tests

```bat
cl /nologo /W4 /O2 /I src /c src\LinearAlgebraNaive.c /Fosrc\LinearAlgebraNaive.obj

cl /nologo /W4 /O2 /I src /c src\Tests.c /Fosrc\Tests.obj

link /OUT:Tests.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 src\LinearAlgebraNaive.obj src\Tests.obj src\LinearAlgebra.obj

Tests.exe
```

### Build and run benchmarks

```bat
cl /nologo /W4 /O2 /I src /c src\Benchmark.c /Fosrc\Benchmark.obj

link /OUT:Benchmark.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 src\LinearAlgebraNaive.obj src\Benchmark.obj src\LinearAlgebra.obj

Benchmark.exe
```

## Implemented Operations

### Dot Product

Computes the dot product of two vectors:

$$
A \cdot B = \sum_{i=0}^{n-1} A_i B_i
$$

The assembly implementation uses AVX2 vector operations and FMA instructions to process multiple elements per iteration.

### Matrix Multiplication

Computes the product of two matrices:

$$
C = A \times B
$$

For matrices with compatible dimensions:

$$
A_{m \times n} \times B_{n \times p} = C_{m \times p}
$$

The assembly implementation uses SIMD vectorization and 4×4 register blocking to calculate multiple output elements concurrently.

## Goals

- Further optimize matrix multiplication through cache blocking.
- Improve instruction scheduling and loop unrolling.
- Implement multiple accumulators for dot product.
- Compare hand-written assembly against AVX2-optimized C.
- Expand benchmarks to larger matrix dimensions.