// Function Description:
//  CUDA Error Handler

// Libraries, Includes, etc...
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

// Host Application
inline static cudaError_t checkCuda ( 
    // Port List
    cudaError_t result, 
    const char * srcStr
) {
    // Variables


    // Execution
    if ( result != cudaSuccess ) {
        fprintf ( stderr , "CUDA Runtime Error ( src : %s): %s \n" , srcStr , cudaGetErrorString ( result ));
        // Stop on errors in both Debug and Release builds.
        exit ( EXIT_FAILURE );
    }
    return result;
}
