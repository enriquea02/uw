// Function Description:
//  1D vector manipulation. SAXPY = single-precision Y = (A * X) + Y.
//  All terms are 32-bit floats. Vectors X and Y must be the same width.
//  Output is a 32-bit vector.
// 
//  ICD:
//      i_vector_width: integer value of X and Y vector width
//      scalarA: scalar float multiplier value
//      vectorX: vector of length 1xN
//      vectorY: vector of length 1xN

// Libraries, Includes, etc...
#include "cuda_runtime.h"

// Kernel
__global__ void saxPyKernel(
    // Port List
    int i_vector_width,
    float i_scalarA,
    float *vectorX,
    float *vectorY
) {
    // Assertions


    // Variables
    int tid = blockIdx.x * blockDim.x + threadIdx.x;    

    // Execution


    // Output
    if (tid > -1 &&
        tid < i_vector_width) {
        // Valid thread
        vectorY[tid] = (i_scalarA * vectorX[tid]) + vectorY[tid];
    }
} 