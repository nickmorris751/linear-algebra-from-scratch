#ifndef LINEARALGEBRA_H
#define LINEARALGEBRA_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

double CDotProduct(const double* vectorA, const double* vectorB, size_t size);
double ASMDotProduct(const double* vectorA, const double* vectorB, size_t size);
int CMatrixMultiply(const double* matrixA, const double* matrixB, double* matrixC, size_t rowsA, size_t columnsA, size_t columnsB);
int ASMMatrixMultiply(const double* matrixA, const double* matrixB, double* matrixC, size_t rowsA, size_t columnsA, size_t columnsB);


#ifdef __cplusplus
}
#endif

#endif