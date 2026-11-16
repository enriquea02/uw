// Function Description:
//  This kernel performs matrix multiplication of two rectangular matrices M and N,
//  producing the output matrix P. Matrices are not necessarily square.
// 
//  ICD:
//

// Libraries, Includes, etc...
#include "mmult.h"

// Kernel
__global__ void mmult_sqr_gmem(
    // Port List
    float* M, // input matrix M (J x K)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J, // number of rows in M and P
    int K, // number of columns in M and rows in N
    int L  // number of columns in N and P
) {
    // Assertions

    
    // Variables
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    // Execution    
    if ((row < J) && (col < L))
    {
        float pval = 0;
        for (int k = 0; k < K; ++k)
        {
            pval += M[row * K + k] * N[k * L + col];
        }
        P[row * L + col] = pval;
    }
}

__global__ void tiled_mmult(
    // Port List
    float* M, // input matrix M (J x K)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J, // number of rows in M and P
    int K, // number of columns in M and rows in N
    int L  // number of columns in N and P
) {
    // Assertions

    // Variables
    __shared__ float ds_M[TILE_WIDTH][TILE_WIDTH];
    __shared__ float ds_N[TILE_WIDTH][TILE_WIDTH];
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = threadIdx.y;
    int tx = threadIdx.x;
    float pval = 0;
    int p = 0;

    // Execution
    for (p = 0; p < (K + TILE_WIDTH - 1) / TILE_WIDTH; ++p) {
        // Load the tile of M into shared memory
        //  Each thread needs to test
        //      • (Row < M_HEIGHT) && (p*TILE_WIDTH+tx < Width)
        //          • If true, load M element
        //      • Else , load 0
        if((row < J) && (p * TILE_WIDTH+tx < K)) {
            ds_M[ty][tx] = M[row * K + (p * TILE_WIDTH + tx)];
        } else {
            ds_M[ty][tx] = 0.0;
        }
        __syncthreads();
        
        // Load the tile of N into shared memory
        //  Each thread needs to test
        //      • (p*TILE_WIDTH+ty < K) && (col < L)
        //          • If true, load N element
        //      • Else , load 0
        if((p * TILE_WIDTH+ty < K) && (col < L)) {
            ds_N[ty][tx] = N[(p * TILE_WIDTH + ty) * L + col];
        } else {
            ds_N[ty][tx] = 0.0;
        }
        __syncthreads();

        // Compute the partial sum for the current tile
        if ((row < J) && (col < L))
        {
            for (int k = 0; k < TILE_WIDTH; ++k)
            {
                pval += ds_M[ty][k] * ds_N[k][tx];
            }
        }
        __syncthreads();
    }

    // Place the result into global memory
    if ((row < J) && (col < L))
    {
        P[row * L + col] = pval;
    }
}

__global__ void cornerTurning_mmult(
    // Port List
    float* M, // input matrix M (K x J)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J, // number of columns in M and rows in P
    int K, // number of rows in M and rows in N
    int L  // number of columns in N and P
) {
    // Assertions

    // Variables
    __shared__ float ds_M[TILE_WIDTH][TILE_WIDTH];
    __shared__ float ds_N[TILE_WIDTH][TILE_WIDTH];
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int ty = threadIdx.y;
    int tx = threadIdx.x;
    float pval = 0;
    int tileIndex = 0;
    int mRow = 0;
    int mCol = 0;

    // Execution
    for (tileIndex = 0; tileIndex < ((K + (TILE_WIDTH - 1)) / TILE_WIDTH); ++tileIndex) {
        // Load the tile from the row-major M into shared memory
        //  Each thread needs to test
        //      • (mRow < K) && (mCol < J)
        //          • If true, load M element
        //      • Else , load 0
        mRow = tileIndex * TILE_WIDTH + ty;
        mCol = blockIdx.y * TILE_WIDTH + tx;
        if((mRow < K) && (mCol < J)) {
            ds_M[ty][tx] = M[(mRow * J) + mCol]; 
        } else {
            ds_M[ty][tx] = 0.0;
        }
        __syncthreads();
        
        // Load the tile of N into shared memory
        //  Each thread needs to test
        //      • (tileIndex*TILE_WIDTH+ty < K) && (col < L)
        //          • If true, load N element
        //      • Else , load 0
        if((tileIndex * TILE_WIDTH+ty < K) && (col < L)) {
            ds_N[ty][tx] = N[(tileIndex * TILE_WIDTH + ty) * L + col];
        } else {
            ds_N[ty][tx] = 0.0;
        }
        __syncthreads();

        // Compute the partial sum for the current tile
        if ((row < J) && (col < L))
        {
            for (int k = 0; k < TILE_WIDTH; ++k)
            {
                pval += ds_M[k][ty] * ds_N[k][tx];
            }
        }
        __syncthreads();
    }

    // Place the result into global memory
    if ((row < J) && (col < L))
    {
        P[row * L + col] = pval;
    }
}