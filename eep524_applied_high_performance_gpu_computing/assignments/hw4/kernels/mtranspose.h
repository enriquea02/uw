// Function Description:
//  Matrix Transpose Parameters and Kernel Declarations
// 
//  ICD:
// 

#ifndef HW4_MTRANPOSE_H
#define HW4_MTRANPOSE_H

// ---- Libraries, Includes, etc...
#include <cuda_runtime.h>

// ---- Parameters
// TILE_DIM is being set as a compiler flag so I can 
// easily change it without modifying the source code :)
#ifndef TILE_DIM
#define TILE_DIM 32 // Setting TILE_DIM as a default if it is not already defined.
#endif

#ifndef BLOCK_ROWS
#define BLOCK_ROWS 32 // Setting BLOCK_ROWS as a default if it is not already defined.
#endif

// Kernel Prototypes
__global__ void transposeCoalesced(
    // Port List
    const float* idata, // input matrix M (J x K)
    float* odata        // output matrix P (K x J)
);

#endif
