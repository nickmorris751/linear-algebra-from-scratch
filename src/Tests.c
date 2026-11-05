#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "LinearAlgebra.h"

#define TOLERANCE 1e-9

static int NearlyEqual(double A, double B){
    return fabs(A - B) <= TOLERANCE * fmax(1.0, fmax(fabs(A), fabs(B)));
}

static int TestDotProduct(size_t Size){
    double* VectorA = malloc(Size * sizeof(double));
    double* VectorB = malloc(Size * sizeof(double));

    if (VectorA == NULL || VectorB == NULL){
        free(VectorA);
        free(VectorB);
        return 0;
    }

    for (size_t i = 0; i < Size; i++){
        VectorA[i] = (double)(i % 100) * 0.5;
        VectorB[i] = (double)(i % 50) * 0.25;
    }
    double CResult = CDotProduct(VectorA, VectorB, Size);
    double ASMResult = ASMDotProduct(VectorA, VectorB, Size);
    int Passed = NearlyEqual(CResult, ASMResult);
    free(VectorA);
    free(VectorB);
    return Passed;
}

static int TestMatrixMultiply(size_t RowsA, size_t ColumnsA, size_t ColumnsB){
    size_t SizeA = RowsA * ColumnsA;
    size_t SizeB = ColumnsA * ColumnsB;
    size_t SizeC = RowsA * ColumnsB;
    double* MatrixA = malloc(SizeA * sizeof(double));
    double* MatrixB = malloc(SizeB * sizeof(double));
    double* CResult = calloc(SizeC, sizeof(double));
    double* ASMResult = calloc(SizeC, sizeof(double));
    if (!MatrixA || !MatrixB || !CResult|| !ASMResult){
        free(MatrixA);
        free(MatrixB);
        free(CResult);
        free(ASMResult);
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

    int Passed = CSuccess && ASMSuccess;

    for (size_t i = 0; i < SizeC && Passed; i++){
        if (!NearlyEqual(CResult[i], ASMResult[i])){Passed = 0;}
    }

    free(MatrixA);
    free(MatrixB);
    free(CResult);
    free(ASMResult);

    return Passed;
}

int main(int argc, char** argv){
    int AllPassed = 1;

    const size_t VectorSizes[] = {1, 3, 4, 5, 16, 1003, 1000000};

    const size_t VectorTestCount = sizeof(VectorSizes) / sizeof(VectorSizes[0]);

    printf("Dot Product Tests\n\n");

    for (size_t i = 0; i < VectorTestCount; i++){
        int Passed = TestDotProduct(VectorSizes[i]);
        printf("Size %zu: %s\n", VectorSizes[i], Passed ? "PASS" : "FAIL");

        if (!Passed){AllPassed = 0;}
    }

    printf("\nMatrix Multiplication Tests\n\n");

    const size_t MatrixSizes[][3] = {
        {1, 1, 1},
        {2, 3, 4},
        {3, 5, 2},
        {4, 4, 4},
        {5, 7, 3},
        {16, 16, 16}
    };

    const size_t MatrixTestCount = sizeof(MatrixSizes) / sizeof(MatrixSizes[0]);

    for (size_t i = 0; i < MatrixTestCount; i++){
        size_t RowsA = MatrixSizes[i][0];
        size_t ColumnsA = MatrixSizes[i][1];
        size_t ColumnsB = MatrixSizes[i][2];
        int Passed = TestMatrixMultiply(RowsA, ColumnsA, ColumnsB);
        printf("%zux%zu * %zux%zu: %s\n", RowsA, ColumnsA, ColumnsA, ColumnsB, Passed ? "PASS" : "FAIL");
        if (!Passed){AllPassed = 0;}
    }

    printf("\nOverall result: %s\n", AllPassed ? "PASS" : "FAIL");

    return AllPassed ? 0 : 1;
}


/*
ml64 /c /Fo src\LinearAlgebra.obj src\LinearAlgebra.asm
cl /nologo /W4 /O2 /I src /c src\LinearAlgebraNaive.c /Fosrc\LinearAlgebraNaive.obj
cl /nologo /W4 /O2 /I src /c src\Tests.c /Fosrc\Tests.obj
link /OUT:Tests.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 src\LinearAlgebraNaive.obj src\Tests.obj src\LinearAlgebra.obj
Tests.exe

*/