// Function Description:
//  Run CPU, global-memory GPU, and tiled GPU matrix multiplication.
//  Compare both GPU results against the CPU result for each case.
//
//  ICD:
//  M: J x K, 
//  N: K x L, 
//  P: J x L


// Libraries, Includes, etc...
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "checkCuda.h"
#include "../kernels/mmult.h"

// Host Functions
void mmult(float* M, float* N, float* P, int J, int K, int L);
int verify_mmult(const char* name, float* cpu, float* gpu, int count,
                 float relative_tolerance, float zero_tolerance);

// host
int main(
    // Port List
) {
    // ---- define/initialize static/const variables
    const int rows[] = {100, 1000, 2000};
    const int columns[] = {100, 1000, 3000};
    const int inner_sizes[] = {100, 1000, 2500};
    const float relative_tolerance = 1e-4f;
    const float zero_tolerance = 1e-5f; // used only when the CPU result is exactly zero

    // ---- variables
    int J = 0;
    int K = 0;
    int L = 0;
    int total_errors = 0;
    size_t bytes_M = 0;
    size_t bytes_N = 0;
    size_t bytes_P = 0;

    // ---- pointers from host memory
    float* M_h = nullptr;
    float* N_h = nullptr;
    float* P_cpu = nullptr;
    float* P_naive = nullptr;
    float* P_tiled = nullptr;

    // ---- pointers from device memory
    float* M_d = nullptr;
    float* N_d = nullptr;
    float* P_d = nullptr;

    // ---- initialize GPU host API
    checkCuda(cudaSetDevice(0), "cudaSetDevice");

    // ---- run all matrix sizes
    for (int caseIndex = 0; caseIndex < 3; ++caseIndex) {
        J = rows[caseIndex];
        K = inner_sizes[caseIndex];
        L = columns[caseIndex];
        bytes_M = (size_t)J * K * sizeof(float);
        bytes_N = (size_t)K * L * sizeof(float);
        bytes_P = (size_t)J * L * sizeof(float);

        printf("\nOutput: %d x %d, inner size: %d, tile width: %d\n",
               J, L, K, TILE_WIDTH);
        printf("Tolerances: relative = %g, zero reference = %g\n",
               relative_tolerance, zero_tolerance);

        // ---- allocate host memory variables
        M_h = (float*)malloc(bytes_M);
        N_h = (float*)malloc(bytes_N);
        P_cpu = (float*)malloc(bytes_P);
        P_naive = (float*)malloc(bytes_P);
        P_tiled = (float*)malloc(bytes_P);

        if (M_h     == nullptr || 
            N_h     == nullptr || 
            P_cpu   == nullptr ||
            P_naive == nullptr || 
            P_tiled == nullptr)
        {
            fprintf(stderr, "Host memory allocation failed.\n");
            free(M_h);
            free(N_h);
            free(P_cpu);
            free(P_naive);
            free(P_tiled);
            return 1;
        }

        // ---- initialize host inputs
        for (int i = 0; i < J * K; ++i) {
            M_h[i] = 0.01f * (i % 23 + 1);
        }
        for (int i = 0; i < K * L; ++i) {
            N_h[i] = 0.02f * (i % 17 + 1);
        }

        // ---- compute CPU references
        printf("Computing CPU reference...\n");
        fflush(stdout);
        mmult(M_h, N_h, P_cpu, J, K, L);

        // ---- allocate device memory variables
        checkCuda(cudaMalloc(&M_d, bytes_M), "cudaMalloc M");
        checkCuda(cudaMalloc(&N_d, bytes_N), "cudaMalloc N");
        checkCuda(cudaMalloc(&P_d, bytes_P), "cudaMalloc P");

        // ---- copy host memory to device memory
        checkCuda(cudaMemcpy(M_d, M_h, bytes_M, cudaMemcpyHostToDevice), "Copy M");
        checkCuda(cudaMemcpy(N_d, N_h, bytes_N, cudaMemcpyHostToDevice), "Copy N");

        // ---- determine GPU device kernel execution configuration
        dim3 threads(TILE_WIDTH, TILE_WIDTH);
        dim3 blocks((L + TILE_WIDTH - 1) / TILE_WIDTH,
                    (J + TILE_WIDTH - 1) / TILE_WIDTH);

        // ---- launch naive kernel, check errors, and retrieve its result
        checkCuda(cudaMemset(P_d, 0, bytes_P), "Clear naive output");
        mmult_sqr_gmem<<<blocks, threads>>>(M_d, N_d, P_d, J, K, L);
        checkCuda(cudaGetLastError(), "Naive MMULT launch");
        checkCuda(cudaDeviceSynchronize(), "Naive MMULT execution");
        checkCuda(cudaMemcpy(P_naive, P_d, bytes_P, cudaMemcpyDeviceToHost),
                  "Copy naive output");

        // ---- launch tiled kernel, check errors, and retrieve its result
        // Clear the buffer so the naive result cannot hide a missing tiled write.
        checkCuda(cudaMemset(P_d, 0, bytes_P), "Clear tiled output");
        tiled_mmult<<<blocks, threads>>>(M_d, N_d, P_d, J, K, L);
        checkCuda(cudaGetLastError(), "Tiled MMULT launch");
        checkCuda(cudaDeviceSynchronize(), "Tiled MMULT execution");
        checkCuda(cudaMemcpy(P_tiled, P_d, bytes_P, cudaMemcpyDeviceToHost),
                  "Copy tiled output");

        // ---- use/verify results on host
        total_errors += verify_mmult("Naive GPU", P_cpu, P_naive, J * L,
                                     relative_tolerance, zero_tolerance);
        total_errors += verify_mmult("Tiled GPU", P_cpu, P_tiled, J * L,
                                     relative_tolerance, zero_tolerance);

        // ---- free device and host memory
        checkCuda(cudaFree(M_d), "cudaFree M");
        checkCuda(cudaFree(N_d), "cudaFree N");
        checkCuda(cudaFree(P_d), "cudaFree P");
        free(M_h);
        free(N_h);
        free(P_cpu);
        free(P_naive);
        free(P_tiled);
    }

    if (total_errors > 0) {
        return 1;
    }
    return 0;
}

// Function Description:
//  Compute the CPU serial reference result.
//
// host
void mmult(
    // Port List
    float* M, // input matrix M (J x K)
    float* N, // input matrix N (K x L)
    float* P, // output matrix P (J x L)
    int J,
    int K,
    int L
) {
    // ---- compute CPU serial reference result
    for (int row = 0; row < J; ++row) {
        for (int col = 0; col < L; ++col) {
            float pval = 0;
            for (int k = 0; k < K; ++k) {
                pval += M[row * K + k] * N[k * L + col];
            }
            P[row * L + col] = pval;
        }
    }
}

// Function Description:
//  Check every GPU output element and report the largest error fraction.
//
// host
int verify_mmult(
    // Port List
    const char* name,
    float* cpu,
    float* gpu,
    int count,
    float relative_tolerance,
    float zero_tolerance
) {
    // ---- variables
    int error_count = 0;
    int zero_reference_count = 0;
    int nonfinite_count = 0;
    float error = 0;
    float error_fraction = 0;
    float max_error_fraction = 0;

    // ---- compare every output element
    for (int i = 0; i < count; ++i) {
        // Verify result is not NaN or infinite (invalid in any case)
        if (!std::isfinite(cpu[i]) || !std::isfinite(gpu[i])) {
            error_count++;
            nonfinite_count++;
            continue;
        }

        // Compare zero references without dividing by zero.
        error = std::fabs(gpu[i] - cpu[i]);
        if (cpu[i] == 0) {
            zero_reference_count++;
            if (error > zero_tolerance) {
                error_count++;
            }
        } else {
            // Nonzero references use relative error only.
            error_fraction = error / std::fabs(cpu[i]);
            if (error_fraction > relative_tolerance) {
                error_count++;
            }
            if (error_fraction > max_error_fraction) {
                max_error_fraction = error_fraction;
            }
        }
    }

    // ---- print verification results
    if (error_count == 0) {
        printf("%s: PASS\n", name);
    } else {
        printf("%s: FAIL\n", name);
    }
    printf("  Mismatches: %d out of %d\n", error_count, count);
    printf("  Maximum error fraction (finite, nonzero CPU values): %g\n",
           max_error_fraction);
    printf("  Zero CPU values checked with zero tolerance: %d\n",
           zero_reference_count);
    printf("  Non-finite output pairs: %d\n", nonfinite_count);

    return error_count;
}
