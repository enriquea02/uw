// Function Description:
//  3D volume grid addition. All terms are 32-bit floats.
//  Volume grids X and Y must be the same dimensions (M x N x P).
//  Output is a 3D volume grid of the same dimensions. Kernel does bound
//  checking to ensure that threads are valid for the
//  given volume grid dimensions.
// 
//  ICD:
//    i_m_rows: integer value of M input volume grid rows
//    i_n_cols: integer value of N input volume grid columns
//    i_p_depth: integer value of P input volume grid depth
//    volumeGridX: volume grid of dimensions M x N x P
//    volumeGridY: volume grid of dimensions M x N x P
//    volumeGridZ: output volume grid of dimensions M x N x P

// Libraries, Includes, etc...

#include "cuda_runtime.h"

// Kernel
__global__ void volumeGridAdditionKernel(
    // Port List
    int i_m_rows,
    int i_n_cols,
    int i_p_depth,
    float *volumeGridX,
    float *volumeGridY,
    float *volumeGridZ
) {
    // Assertions

    
    // Variables
    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    int Depth = blockIdx.z * blockDim.z + threadIdx.z;
    int idx = (Depth * (i_m_rows * i_n_cols)) + (Row * i_n_cols) + Col;

    // Execution
    
    
    // Output
    if (Col > -1 && Col < i_n_cols &&
        Row > -1 && Row < i_m_rows &&
        Depth > -1 && Depth < i_p_depth
    ) {
        // Valid thread
        volumeGridZ[idx] = volumeGridX[idx] + volumeGridY[idx];
    }
} 