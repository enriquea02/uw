// Function Description:
//  Control Divergence Examples
// 
//  ICD:

// Libraries, Includes, etc...

#include "control_divergence.h"

// Kernel
__global__ void SingleBifurcation(
    // Port List
    float *data,
    bool divergent
) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if ((threadIdx.x < 10) || (!divergent)) {
        data[index] = threadIdx.x;
    } else {
        data[index] = threadIdx.x * 2;
    }

} 

// Kernel
__global__ void MultipleBifurcation(
    // Port List
    float *data,
    bool divergent
) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;

    if ((threadIdx.x < 10) || (!divergent)) {
        data[index] = threadIdx.x;
    } else if (threadIdx.x < 20) {
        data[index] = threadIdx.x * 2;
    } else {
        data[index] = threadIdx.x * 3;
    }

} 
