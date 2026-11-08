// Function Description:
//  Main Host Application - Control Divergence
// 
//  ICD:
// 

// Libraries, Includes, etc...
#include <cuda_runtime.h>                 // CUDA Runtime API and dim3
#include "checkCuda.h"                    //  Lecture 1 error-checking function
#include "../kernels/control_divergence.h" // Declarations of kernel functions

// host
int main(
    // Port List
) {
    // ---- define/initialize static/const variables
    // control divergence parameters
    const int threads = 32;  // one warp per block
    const int blocks = 256;
    const int total_threads = threads * blocks;

    // ---- pointers from device memory
    float *data_d = nullptr;

    // ---- initialize GPU host API
    checkCuda(cudaSetDevice(0), "cudaSetDevice");

    // ---- allocate device output memory
    checkCuda(cudaMalloc(&data_d, total_threads * sizeof(float)), "cudaMalloc");

    // ---- run all kernel test cases
    // single branch: with divergence
    SingleBifurcation<<<blocks, threads>>>(data_d, true);
    checkCuda(cudaGetLastError(), "SingleBifurcation divergent launch");
    checkCuda(cudaDeviceSynchronize(), "SingleBifurcation divergent execution");

    // single branch: without divergence
    SingleBifurcation<<<blocks, threads>>>(data_d, false);
    checkCuda(cudaGetLastError(), "SingleBifurcation uniform launch");
    checkCuda(cudaDeviceSynchronize(), "SingleBifurcation uniform execution");

    // multiple branches: with divergence
    MultipleBifurcation<<<blocks, threads>>>(data_d, true);
    checkCuda(cudaGetLastError(), "MultipleBifurcation divergent launch");
    checkCuda(cudaDeviceSynchronize(), "MultipleBifurcation divergent execution");

    // multiple branches: without divergence
    MultipleBifurcation<<<blocks, threads>>>(data_d, false);
    checkCuda(cudaGetLastError(), "MultipleBifurcation uniform launch");
    checkCuda(cudaDeviceSynchronize(), "MultipleBifurcation uniform execution");

    // ---- free device output memory
    checkCuda(cudaFree(data_d), "cudaFree");

    return 0;
}
