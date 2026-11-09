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
        for (int i = 0; i < 10; i++) {
            data[index] = (data[index] * threadIdx.x) + 1.5f;
        }
    } else {
        for (int i = 0; i < 10; i++) {
            data[index] = ((data[index] + 3.0f) * 0.501f) - 1.5f;
        }    }

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
