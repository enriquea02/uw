// Function Description:
//  CUDA Error Handler

// Libraries, Includes, etc...
#include <stdio.h>
#include <assert.h>
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
        assert ( result == cudaSuccess );
    }
    return result;
}