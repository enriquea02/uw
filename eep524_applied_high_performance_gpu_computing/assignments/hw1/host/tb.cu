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
// Run each required printf launch configuration ten times.
void tbPrintFKernel()
{
    const int blockCounts[] = {1, 4, 32};
    const int threadCounts[] = {32, 8, 1};
    const int repetitions = 10;

    for (int caseIndex = 0; caseIndex < 3; ++caseIndex) {
        const int blocks = blockCounts[caseIndex];
        const int threads = threadCounts[caseIndex];

        for (int run = 0; run < repetitions; ++run) {
            printf("\nprintf kernel <<<%d, %d>>>: run %d of %d\n",
                   blocks, threads, run + 1, repetitions);

            printFKernel<<<blocks, threads>>>();
            checkCuda(cudaGetLastError(), "printFKernel launch");
            checkCuda(cudaDeviceSynchronize(), "printFKernel execution");
        }
    }
}

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

void tbMatrixAdditionKernel(int M, int N)
{
    size_t count = (size_t)M * N;
    size_t bytes = count * sizeof(float);
    float rTol = 1e-5f;
    float aTol = 1e-6f;
    size_t errorCount = 0;

    printf("Running Matrix addition: %dx%d\n", M, N);

    // ---- allocate host memory (ordinary memory avoids pinning large volumes)
    float *X_h = (float *)malloc(bytes);
    float *Y_h = (float *)malloc(bytes);
    float *Z_h = (float *)malloc(bytes);
    float *cpuResult = (float *)malloc(bytes);

    if (!X_h || !Y_h || !Z_h || !cpuResult) {
        fprintf(stderr, "Matrix addition: host allocation failed\n");
        free(X_h);
        free(Y_h);
        free(Z_h);
        free(cpuResult);
        exit(EXIT_FAILURE);
    }

    // ---- initialize inputs and compute the sequential CPU reference
    for (size_t i = 0; i < count; ++i) {
        X_h[i] = 0.1f * (i % 1000);
        Y_h[i] = 0.2f * ((i + 7) % 1000);
        cpuResult[i] = X_h[i] + Y_h[i];
    }

    // ---- allocate device memory
    float *X_d = nullptr;
    float *Y_d = nullptr;
    float *Z_d = nullptr;
    checkCuda(cudaMalloc(&X_d, bytes), "cudaMalloc X");
    checkCuda(cudaMalloc(&Y_d, bytes), "cudaMalloc Y");
    checkCuda(cudaMalloc(&Z_d, bytes), "cudaMalloc Z");

    // ---- copy inputs to the device
    checkCuda(cudaMemcpy(X_d, X_h, bytes, cudaMemcpyHostToDevice), "cudaMemcpy X");
    checkCuda(cudaMemcpy(Y_d, Y_h, bytes, cudaMemcpyHostToDevice), "cudaMemcpy Y");

    // ---- x maps to columns, y maps to rows
    dim3 threads(16, 16);
    dim3 blocks((N + threads.x - 1) / threads.x,
                (M + threads.y - 1) / threads.y);

    // ---- launch and check execution
    MatrixAdditionKernel<<<blocks, threads>>>(M, N, X_d, Y_d, Z_d);
    checkCuda(cudaGetLastError(), "MatrixAdditionKernel launch");
    checkCuda(cudaDeviceSynchronize(), "MatrixAdditionKernel execution");

    // ---- retrieve and verify every output element
    checkCuda(cudaMemcpy(Z_h, Z_d, bytes, cudaMemcpyDeviceToHost), "cudaMemcpy Z");
    for (size_t i = 0; i < count; ++i) {
        float error = abs(cpuResult[i] - Z_h[i]);
        float limit = max(aTol, rTol * max(abs(cpuResult[i]), abs(Z_h[i])));
        if (!(error <= limit)) {
            ++errorCount;
        }
    }
    printf("Matrix addition %dx%d: %s (%zu mismatches)\n",
           M, N, errorCount == 0 ? "PASS" : "FAIL", errorCount);

    // ---- release device and host memory
    checkCuda(cudaFree(X_d), "cudaFree X");
    checkCuda(cudaFree(Y_d), "cudaFree Y");
    checkCuda(cudaFree(Z_d), "cudaFree Z");
    free(X_h);
    free(Y_h);
    free(Z_h);
    free(cpuResult);
}


void tbVolumeGridAdditionKernel(int M, int N, int P)
{
    const size_t count = (size_t)M * N * P;
    const size_t bytes = count * sizeof(float);
    const float rTol = 1e-5f;
    const float aTol = 1e-6f;
    size_t errorCount = 0;

    printf("Running Volume addition: %dx%dx%d\n", M, N, P);

    // ---- allocate host memory (ordinary memory avoids pinning large volumes)
    float *X_h = (float *)malloc(bytes);
    float *Y_h = (float *)malloc(bytes);
    float *Z_h = (float *)malloc(bytes);
    float *cpuResult = (float *)malloc(bytes);

    if (!X_h || !Y_h || !Z_h || !cpuResult) {
        fprintf(stderr, "Volume addition: host allocation failed\n");
        free(X_h);
        free(Y_h);
        free(Z_h);
        free(cpuResult);
        exit(EXIT_FAILURE);
    }

    // ---- initialize inputs and compute the sequential CPU reference
    for (size_t i = 0; i < count; ++i) {
        X_h[i] = 0.1f * (i % 1000);
        Y_h[i] = 0.2f * ((i + 7) % 1000);
        cpuResult[i] = X_h[i] + Y_h[i];
    }

    // ---- allocate device memory
    float *X_d = nullptr;
    float *Y_d = nullptr;
    float *Z_d = nullptr;
    checkCuda(cudaMalloc(&X_d, bytes), "cudaMalloc X");
    checkCuda(cudaMalloc(&Y_d, bytes), "cudaMalloc Y");
    checkCuda(cudaMalloc(&Z_d, bytes), "cudaMalloc Z");

    // ---- copy inputs to the device
    checkCuda(cudaMemcpy(X_d, X_h, bytes, cudaMemcpyHostToDevice), "cudaMemcpy X");
    checkCuda(cudaMemcpy(Y_d, Y_h, bytes, cudaMemcpyHostToDevice), "cudaMemcpy Y");

    // ---- x maps to columns, y maps to rows, z maps to depth
    dim3 threads(8, 8, 8);
    dim3 blocks((N + threads.x - 1) / threads.x,
                (M + threads.y - 1) / threads.y,
                (P + threads.z - 1) / threads.z);

    // ---- launch and check execution
    volumeGridAdditionKernel<<<blocks, threads>>>(M, N, P, X_d, Y_d, Z_d);
    checkCuda(cudaGetLastError(), "volumeGridAdditionKernel launch");
    checkCuda(cudaDeviceSynchronize(), "volumeGridAdditionKernel execution");

    // ---- retrieve and verify every output element
    checkCuda(cudaMemcpy(Z_h, Z_d, bytes, cudaMemcpyDeviceToHost), "cudaMemcpy Z");
    for (size_t i = 0; i < count; ++i) {
        float error = abs(cpuResult[i] - Z_h[i]);
        float limit = max(aTol, rTol * max(abs(cpuResult[i]), abs(Z_h[i])));
        if (!(error <= limit)) {
            ++errorCount;
        }
    }
    printf("Volume addition %dx%dx%d: %s (%zu mismatches)\n",
           M, N, P, errorCount == 0 ? "PASS" : "FAIL", errorCount);

    // ---- release device and host memory
    checkCuda(cudaFree(X_d), "cudaFree X");
    checkCuda(cudaFree(Y_d), "cudaFree Y");
    checkCuda(cudaFree(Z_d), "cudaFree Z");
    free(X_h);
    free(Y_h);
    free(Z_h);
    free(cpuResult);
}
