#include "LinearAlgebra.h"


double CDotProduct(const double* vectorA, const double* vectorB, size_t size){
    if (!vectorA || !vectorB || size == 0){return 0.0;}
    double result = 0.0;
    for (size_t i = 0; i < size; i++){
        result += vectorA[i] * vectorB[i];
    }
    return result;
}

int CMatrixMultiply(const double* matrixA, const double* matrixB, double* matrixC, size_t rowsA, size_t columnsA, size_t columnsB){
    if (!matrixA || !matrixB || !matrixC || rowsA == 0 || columnsA == 0 || columnsB == 0){return 0;}
    for (size_t i = 0; i < rowsA; i++){
        for (size_t j = 0; j < columnsB; j++){
            double sum = 0.0;
            for (size_t k = 0; k < columnsA; k++){
                sum += matrixA[i * columnsA + k] * matrixB[k * columnsB + j];
            }
            matrixC[i * columnsB + j] = sum;
        }
    }
    return 1;
}