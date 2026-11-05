#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "LinearAlgebra.h"

#define SAMPLE_COUNT 9
#define DOT_PRODUCT_SIZE 262144
#define DOT_PRODUCT_ITERATIONS 100
#define MATRIX_SIZE 512
#define MATRIX_ITERATIONS 20
#define TOLERANCE 1e-9

typedef double (*DotProductFunction)(const double*, const double*, size_t);

typedef int (*MatrixMultiplyFunction)(const double*, const double*, double*, size_t, size_t, size_t);

static volatile double BenchmarkSink = 0.0;

static int NearlyEqual(double A, double B){
    return fabs(A - B) <= TOLERANCE * fmax(1.0, fmax(fabs(A), fabs(B)));
}

static double GetElapsedSeconds(LARGE_INTEGER Start, LARGE_INTEGER End, LARGE_INTEGER Frequency){
    return (double)(End.QuadPart - Start.QuadPart) / (double)Frequency.QuadPart;
}

static double BenchmarkDotProduct(DotProductFunction Function, const double* VectorA, const double* VectorB, size_t Size, size_t Iterations, LARGE_INTEGER Frequency){
    LARGE_INTEGER Start;
    LARGE_INTEGER End;
    double Result = 0.0;

    QueryPerformanceCounter(&Start);

    for (size_t i = 0; i < Iterations; i++){
        Result = Function(VectorA, VectorB, Size);
    }

    QueryPerformanceCounter(&End);

    BenchmarkSink = Result;

    return GetElapsedSeconds(Start, End, Frequency) / (double)Iterations;
}

static double BenchmarkMatrixMultiply(MatrixMultiplyFunction Function, const double* MatrixA, const double* MatrixB, double* MatrixC, size_t RowsA, size_t ColumnsA, size_t ColumnsB, size_t Iterations, LARGE_INTEGER Frequency){
    LARGE_INTEGER Start;
    LARGE_INTEGER End;

    QueryPerformanceCounter(&Start);

    for (size_t i = 0; i < Iterations; i++){
        if (!Function(MatrixA, MatrixB, MatrixC, RowsA, ColumnsA, ColumnsB)){
            return -1.0;
        }
    }

    QueryPerformanceCounter(&End);

    BenchmarkSink = MatrixC[0];

    return GetElapsedSeconds(Start, End, Frequency) / (double)Iterations;
}

static void SortSamples(double Samples[SAMPLE_COUNT]){
    for (size_t i = 0; i < SAMPLE_COUNT - 1; i++){
        for (size_t j = i + 1; j < SAMPLE_COUNT; j++){
            if (Samples[j] < Samples[i]) {
                double Temp = Samples[i];
                Samples[i] = Samples[j];
                Samples[j] = Temp;
            }
        }
    }
}

static void PrintResults(const char* Name, double CTime, double ASMTime){
    printf("%s\n", Name);
    printf("  C:        %.6f ms\n", CTime * 1000.0);
    printf("  Assembly: %.6f ms\n", ASMTime * 1000.0);

    if (ASMTime > 0.0){
        printf("  Speedup:  %.2fx\n\n", CTime / ASMTime);
    }
    else{
        printf("  Speedup:  unavailable\n\n");
    }
}

static int RunDotProductBenchmark(LARGE_INTEGER Frequency){
    const size_t Size = DOT_PRODUCT_SIZE;

    double* VectorA = malloc(Size * sizeof(double));
    double* VectorB = malloc(Size * sizeof(double));

    if (!VectorA || !VectorB){
        free(VectorA);
        free(VectorB);
        printf("Dot product allocation failed.\n");
        return 0;
    }

    for (size_t i = 0; i < Size; i++){
        VectorA[i] = (double)(i % 100) * 0.5;
        VectorB[i] = (double)(i % 50) * 0.25;
    }

    double CResult = CDotProduct(VectorA, VectorB, Size);
    double ASMResult = ASMDotProduct(VectorA, VectorB, Size);

    if (!NearlyEqual(CResult, ASMResult)){
        printf("Dot product implementations produced different results.\n");
        free(VectorA);
        free(VectorB);
        return 0;
    }

    for (size_t i = 0; i < 5; i++){
        BenchmarkDotProduct(CDotProduct, VectorA, VectorB, Size, 1, Frequency);
        BenchmarkDotProduct(ASMDotProduct, VectorA, VectorB, Size, 1, Frequency);
    }

    double CSamples[SAMPLE_COUNT];
    double ASMSamples[SAMPLE_COUNT];

    for (size_t i = 0; i < SAMPLE_COUNT; i++){
        if (i % 2 == 0){
            CSamples[i] = BenchmarkDotProduct(CDotProduct, VectorA, VectorB, Size, DOT_PRODUCT_ITERATIONS, Frequency);
            ASMSamples[i] = BenchmarkDotProduct(ASMDotProduct, VectorA, VectorB, Size, DOT_PRODUCT_ITERATIONS, Frequency);
        }
        else{
            ASMSamples[i] = BenchmarkDotProduct(ASMDotProduct, VectorA, VectorB, Size, DOT_PRODUCT_ITERATIONS, Frequency);
            CSamples[i] = BenchmarkDotProduct(CDotProduct, VectorA, VectorB, Size, DOT_PRODUCT_ITERATIONS, Frequency);
        }
    }

    SortSamples(CSamples);
    SortSamples(ASMSamples);

    printf("Dot product: %zu elements, %d iterations per sample\n", Size, DOT_PRODUCT_ITERATIONS);

    PrintResults("Median time per operation", CSamples[SAMPLE_COUNT / 2], ASMSamples[SAMPLE_COUNT / 2]);

    free(VectorA);
    free(VectorB);

    return 1;
}

static int RunMatrixMultiplyBenchmark(LARGE_INTEGER Frequency){
    const size_t RowsA = MATRIX_SIZE;
    const size_t ColumnsA = MATRIX_SIZE;
    const size_t ColumnsB = MATRIX_SIZE;

    const size_t SizeA = RowsA * ColumnsA;
    const size_t SizeB = ColumnsA * ColumnsB;
    const size_t SizeC = RowsA * ColumnsB;

    double* MatrixA = malloc(SizeA * sizeof(double));
    double* MatrixB = malloc(SizeB * sizeof(double));
    double* CResult = malloc(SizeC * sizeof(double));
    double* ASMResult = malloc(SizeC * sizeof(double));

    if (!MatrixA || !MatrixB || !CResult || !ASMResult){
        free(MatrixA);
        free(MatrixB);
        free(CResult);
        free(ASMResult);
        printf("Matrix allocation failed.\n");
        return 0;
    }

    for (size_t i = 0; i < SizeA; i++){
        MatrixA[i] = (double)(i % 10) * 0.5;
    }

    for (size_t i = 0; i < SizeB; i++){
        MatrixB[i] = (double)(i % 7) * 0.25;
    }

    int CSuccess = CMatrixMultiply(MatrixA, MatrixB, CResult, RowsA, ColumnsA, ColumnsB);

    int ASMSuccess = ASMMatrixMultiply(MatrixA, MatrixB, ASMResult, RowsA, ColumnsA, ColumnsB);

    if (!CSuccess || !ASMSuccess){
        printf("Matrix multiplication failed.\n");
        free(MatrixA);
        free(MatrixB);
        free(CResult);
        free(ASMResult);
        return 0;
    }

    for (size_t i = 0; i < SizeC; i++){
        if (!NearlyEqual(CResult[i], ASMResult[i])){
            printf("Matrix multiplication implementations produced different results.\n");
            free(MatrixA);
            free(MatrixB);
            free(CResult);
            free(ASMResult);
            return 0;
        }
    }

    for (size_t i = 0; i < 5; i++){
        BenchmarkMatrixMultiply(CMatrixMultiply, MatrixA, MatrixB, CResult, RowsA, ColumnsA, ColumnsB, 1, Frequency);
        BenchmarkMatrixMultiply(ASMMatrixMultiply, MatrixA, MatrixB, ASMResult, RowsA, ColumnsA, ColumnsB, 1, Frequency);
    }

    double CSamples[SAMPLE_COUNT];
    double ASMSamples[SAMPLE_COUNT];

    for (size_t i = 0; i < SAMPLE_COUNT; i++){
        if (i % 2 == 0){
            CSamples[i] = BenchmarkMatrixMultiply(CMatrixMultiply, MatrixA, MatrixB, CResult, RowsA, ColumnsA, ColumnsB, MATRIX_ITERATIONS, Frequency);
            ASMSamples[i] = BenchmarkMatrixMultiply(ASMMatrixMultiply, MatrixA, MatrixB, ASMResult, RowsA, ColumnsA, ColumnsB, MATRIX_ITERATIONS, Frequency);
        } 
        else{
            ASMSamples[i] = BenchmarkMatrixMultiply(ASMMatrixMultiply, MatrixA, MatrixB, ASMResult, RowsA, ColumnsA, ColumnsB, MATRIX_ITERATIONS, Frequency);
            CSamples[i] = BenchmarkMatrixMultiply(CMatrixMultiply, MatrixA, MatrixB, CResult, RowsA, ColumnsA, ColumnsB, MATRIX_ITERATIONS, Frequency);
        }
    }

    SortSamples(CSamples);
    SortSamples(ASMSamples);

    printf("Matrix multiplication: %zux%zu * %zux%zu, %d iterations per sample\n", RowsA, ColumnsA, ColumnsA, ColumnsB, MATRIX_ITERATIONS);

    PrintResults("Median time per operation", CSamples[SAMPLE_COUNT / 2], ASMSamples[SAMPLE_COUNT / 2]);

    free(MatrixA);
    free(MatrixB);
    free(CResult);
    free(ASMResult);

    return 1;
}

int main(int argc, char** argv){
    LARGE_INTEGER Frequency;

    if (!QueryPerformanceFrequency(&Frequency)){
        printf("High-resolution timer unavailable.\n");
        return 1;
    }

    printf("Linear Algebra Benchmark\n");
    printf("========================\n\n");

    int DotProductSuccess = RunDotProductBenchmark(Frequency);
    int MatrixMultiplySuccess = RunMatrixMultiplyBenchmark(Frequency);

    printf("Benchmark result: %s\n", DotProductSuccess && MatrixMultiplySuccess ? "PASS" : "FAIL");

    return DotProductSuccess && MatrixMultiplySuccess ? 0 : 1;
}

/*
ml64 /c /Fo src\LinearAlgebra.obj src\LinearAlgebra.asm

cl /nologo /W4 /O2 /I src /c src\LinearAlgebraNaive.c /Fosrc\LinearAlgebraNaive.obj

cl /nologo /W4 /O2 /I src /c src\Benchmark.c /Fosrc\Benchmark.obj

link /OUT:Benchmark.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 src\LinearAlgebraNaive.obj src\Benchmark.obj src\LinearAlgebra.obj

Benchmark.exe


RESULTS:
Linear Algebra Benchmark
========================

Dot product: 262144 elements, 100 iterations per sample
Median time per operation
  C:        0.122147 ms
  Assembly: 0.066040 ms
  Speedup:  1.85x

Matrix multiplication: 512x512 * 512x512, 20 iterations per sample
Median time per operation
  C:        272.761755 ms
  Assembly: 21.789280 ms
  Speedup:  12.52x

Benchmark result: PASS
*/