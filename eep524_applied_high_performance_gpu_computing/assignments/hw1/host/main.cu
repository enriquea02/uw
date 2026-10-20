// Function Description:
//  Main Host Application
// 
//  ICD:
// 

// Libraries, Includes, etc...
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <cuda_runtime.h>   // CUDA Runtime API and dim3
#include "tb_pkg.h"         // Declarations of your testbench functions
#include "checkCuda.h"      // Your Lecture 1 error-checking macro
#include "kernels_pkg.h"    // Declarations of your kernel functions

// host
int main(
    // Port List
) {
    // ---- define/initialize static/const variables
    // saxpy parameters
    const int saxPy_vector_width[] = {10, 1000, 10000000};
    const float saxPy_a = 2.0f;
    // matrix addition parameters
    const int matrix_m_width[] = {4, 500, 3024};
    const int matrix_n_width[] = {4, 500, 4032};
    // volume grid addition parameters
    const int volumeGrid_m_width[] = {3, 100, 1000};
    const int volumeGrid_n_width[] = {3, 100, 1000};
    const int volumeGrid_p_width[] = {3, 100, 1000};

    // ---- initialize GPU host API
    checkCuda(cudaSetDevice(0), "cudaSetDevice");
    // ---- run all kernel test cases
    tbPrintFKernel();

    for (int N : saxPy_vector_width) {
        tbSaxPyKernel(N, saxPy_a);
    }

    for (int caseIndex = 0; caseIndex < 3; ++caseIndex) {
        tbMatrixAdditionKernel(matrix_m_width[caseIndex],
                               matrix_n_width[caseIndex]);
    }

    // Use a Release build for the largest volume test.
    for (int caseIndex = 0; caseIndex < 3; ++caseIndex) {
        tbVolumeGridAdditionKernel(volumeGrid_m_width[caseIndex],
                                   volumeGrid_n_width[caseIndex],
                                   volumeGrid_p_width[caseIndex]);
    }

    return 0;
}
