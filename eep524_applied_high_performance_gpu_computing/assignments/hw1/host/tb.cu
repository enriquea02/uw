// Function Description:
//  Testbench for CUDA kernels. This testbench will run tests against the following kernels:
//      1.  Printf kernel to print out global thread ID, block ID, and thread ID.
//      2.  1D vector manipulation. SAXPY = single-precision Y = (A * X) + Y.
//      3.  2D matrix addition. All terms are 32-bit floats.
//      4.  3D volume grid addition. All terms are 32-bit floats.

// Libraries, Includes, etc...
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <cmath>
#include <algorithm>

#include <cuda_runtime.h>
#include "checkCuda.h"
#include "kernels_pkg.h"
#include "tb_pkg.h"

using std::max;
using std::abs;

// host tests
void tbSaxPyKernel(
    // Port List
    int N,
    float a
) {
    // Pointers from host memory
    float *X_h = nullptr;
    float *Y_h = nullptr;
    float *cpuResult = nullptr;
    // Pointers from device memory
    float *X_d = nullptr;
    float *Y_d = nullptr;

    // ---- define/initialize static/const variables
    // Kernel dimensions
    const int threads = 256; // Threads per block
    const int blocks = (N + (threads - 1)) / threads;
    // CPU Verification Model
    int errorCount = 0;
    float rTol = 1e-5; // relative tolerance
    float aTol = 1e-6; // absolute tolerance

    // ---- initialize GPU host API
    // ---- query for platform/device information
    // ---- setup GPU host API environment and device program(s)

    // ---- allocate host memory variables _h
    checkCuda(cudaMallocHost(&X_h, N * sizeof(float)), "cudaMallocHost");
    checkCuda(cudaMallocHost(&Y_h, N * sizeof(float)), "cudaMallocHost");
    checkCuda(cudaMallocHost(&cpuResult, N * sizeof(float)), "cudaMallocHost");

    // ---- CPU Verification Model
    for (int i = 0; i < N; ++i) {
        X_h[i] = 2.0f * (i % 1000);
        Y_h[i] = 1.0f * (i % 1000);

        cpuResult[i] = a * X_h[i] + Y_h[i];
    }

    // ---- allocate device memory vars _d
    checkCuda(cudaMalloc(&X_d, N * sizeof(float)), "cudaMalloc");
    checkCuda(cudaMalloc(&Y_d, N * sizeof(float)), "cudaMalloc");


    // ---- copy host memory to device memory
    checkCuda(cudaMemcpy(X_d, X_h, N * sizeof(float), cudaMemcpyDefault), "cudaMemcpy");
    checkCuda(cudaMemcpy(Y_d, Y_h, N * sizeof(float), cudaMemcpyDefault), "cudaMemcpy");

    // ---- determine GPU device kernel execution configuration

    // ---- launch kernel on device
    saxPyKernel<<<blocks, threads>>>(N, a, X_d, Y_d);

    // ---- wait for kernel execution to complete, check for errors
    checkCuda(cudaGetLastError(), "saxPyKernel launch");
    checkCuda(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    // ---- retrieve results from device
    checkCuda(cudaMemcpy(Y_h, Y_d, N * sizeof(float), cudaMemcpyDefault), "cudaMemcpy");

    // ---- use/verify results on host
    for (int i = 0; i < N; ++i) {
        if (abs(cpuResult[i] - Y_h[i]) > max(aTol, rTol * max(abs(cpuResult[i]), abs(Y_h[i])))) {
            errorCount++;
        }
    }

    // ---- free device memory
    checkCuda(cudaFree(X_d), "cudaFree");
    checkCuda(cudaFree(Y_d), "cudaFree");
    checkCuda(cudaFreeHost(X_h), "cudaFreeHost");
    checkCuda(cudaFreeHost(Y_h), "cudaFreeHost");
    checkCuda(cudaFreeHost(cpuResult), "cudaFreeHost");
}