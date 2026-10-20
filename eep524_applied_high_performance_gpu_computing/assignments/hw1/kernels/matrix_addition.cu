// Function Description:
//  2D matrix addition. All terms are 32-bit floats. 
//  Matrices X and Y must be the same MxN dimensions.
//  Output is a MxN 32-bit matrix. Kernel does bound 
//  checking to ensure that threads are valid for the 
//  given matrix dimensions.
// 
//  ICD:
//    i_m_rows: integer value of M input matrix rows
//    i_n_cols: integer value of N input matrix columns
//    matrixX: matrix of dimensions MxN
//    matrixY: matrix of dimensions MxN
//    matrixZ: output matrix of dimensions MxN

// Libraries, Includes, etc...
#include "cuda_runtime.h"

// Kernel
__global__ void MatrixAdditionKernel(
    // Port List
    int i_m_rows,
    int i_n_cols,
    float *matrixX,
    float *matrixY,
    float *matrixZ
) {
    // Assertions

    
    // Variables
    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    int idx = (Row * i_n_cols) + Col;

    // Execution
    
    // Output
    if (Col > -1 &&
        Col < i_n_cols &&
        Row > -1 &&
        Row < i_m_rows
    ) {
        // Valid thread
        matrixZ[idx] = matrixX[idx] + matrixY[idx];
    }
} 