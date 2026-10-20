// Function Description:
//  This kernel prints out global thread ID, block ID, and thread ID.
//  on a single line.

// Libraries, Includes, etc...
#include "cuda_runtime.h"

// Kernel
__global__ void printFKernel(
    // Port List
) {
    // Assertions
    // Variables
    int gid = threadIdx.x + blockDim.x * blockIdx.x;
    
    // Execution    
    printf("Global Thread ID: %d, Block ID: %d, Thread ID: %d\n", gid, blockIdx.x, threadIdx.x);
} 