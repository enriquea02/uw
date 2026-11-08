// Function Description:
//  Control Divergence Kernel Declarations
// 
//  ICD:
// 

#ifndef HW3_CONTROL_DIVERGENCE_H
#define HW3_CONTROL_DIVERGENCE_H

// Libraries, Includes, etc...
#include <cuda_runtime.h>

// Kernel Prototypes
__global__ void SingleBifurcation(float *data, bool divergent);
__global__ void MultipleBifurcation(float *data, bool divergent);

#endif
