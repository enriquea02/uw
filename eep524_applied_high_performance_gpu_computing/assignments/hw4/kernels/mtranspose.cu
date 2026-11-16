// Function Description:
//  This kernel performs matrix transposition of a matrix M through shared memory.
//  It avoids bank conflicts in shared memory and efficiently transposes the matrix 
//  producing the output matrix P. This implementation assumes tile size of 32x8,
//  but I will use 32x32 tiles.
//  
//  Reference: https://developer.nvidia.com/blog/efficient-matrix-transpose-cuda-cc/
// 
//  ICD:
//  M: J x K
//  P: K x J
//  TILE_DIM: 32
//  BLOCK_ROWS: 32

// Libraries, Includes, etc...
#include "cuda_runtime.h"

#define TILE_DIM 32
#define BLOCK_ROWS 32

// Kernel
__global__ void transposeCoalesced(
    // Port List
    const float* idata, // input matrix M (J x K)
    float* odata        // output matrix P (K x J)
) {
    // Assertions

    // Variables
    int x;
    int y;
    int width;

    // Execution
    // Add +1 to tile dimension to eliminate shared memory bank conflicts
    __shared__ float tile[TILE_DIM][TILE_DIM+1]; 

    x = blockIdx.x * TILE_DIM + threadIdx.x;
    y = blockIdx.y * TILE_DIM + threadIdx.y;
    width = gridDim.x * TILE_DIM;

    for (int j = 0; j < TILE_DIM; j += BLOCK_ROWS)
        tile[threadIdx.y+j][threadIdx.x] = idata[(y+j)*width + x];

    // Ensure all threads have finished writing to shared memory before reading
    __syncthreads();

    x = blockIdx.y * TILE_DIM + threadIdx.x;  // transpose block offset
    y = blockIdx.x * TILE_DIM + threadIdx.y;

    // Output
    for (int j = 0; j < TILE_DIM; j += BLOCK_ROWS)
        odata[(y+j)*width + x] = tile[threadIdx.x][threadIdx.y + j];
}