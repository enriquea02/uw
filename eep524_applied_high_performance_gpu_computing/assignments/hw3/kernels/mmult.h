// Function Description:
//  Matrix Multiplication Parameters and Kernel Declarations
// 
//  ICD:
// 

#ifndef HW3_MMULT_H
#define HW3_MMULT_H

// Libraries, Includes, etc...
#include <cuda_runtime.h>

// Parameters
#define TILE_WIDTH 16

// Kernel Prototypes
__global__ void mmult_sqr_gmem(
    // Port List
    float* M, // input matrix M (J x K)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J,
    int K,
    int L
);

__global__ void tiled_mmult(
    // Port List
    float* M, // input matrix M (J x K)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J,
    int K,
    int L
);

#endif
