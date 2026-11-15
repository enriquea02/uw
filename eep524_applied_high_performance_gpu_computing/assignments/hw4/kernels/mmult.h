// Function Description:
//  Matrix Multiplication Parameters and Kernel Declarations
// 
//  ICD:
// 

#ifndef HW3_MMULT_H
#define HW3_MMULT_H

// ---- Libraries, Includes, etc...
#include <cuda_runtime.h>

// ---- Parameters
// TILE_WIDTH is being set as a compiler flag so I can 
// easily change it without modifying the source code :)
#ifndef TILE_WIDTH
#define TILE_WIDTH 16 // Setting TILE_WIDTH as a default if it is not already defined.
#endif

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
