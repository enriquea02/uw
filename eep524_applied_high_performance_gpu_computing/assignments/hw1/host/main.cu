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
    // ---- query for platform/device information
    // ---- setup GPU host API environment and device program(s)
    // ---- allocate host memory variables _h
    // ---- initialize host memory vars
    // ---- allocate device memory vars _d
    // ---- copy host memory to device memory
    // ---- determine GPU device kernel execution configuration
    // ---- launch kernel on device    
    // ---- wait for kernel execution to complete, check for errors
    // ---- retrieve results from device
    // ---- use/verify results on host
}