// Function Description:
// 
// ICD:
// 

// Libraries, Includes, etc...
#include <stdio.h>
#include <assert.h>

// host
int main(
    // Port List
) {
    // define/initialize static/const variables
    // initialize GPU host API
    // query for platform/device information
    // setup GPU host API environment and device program(s)
    // allocate host memory variables _h
    h_imgOut = (unsigned char*)malloc(imgSize);
    // initialize host memory vars
    // allocate device memory vars _d
    cudaMalloc((void**)&d_input, imgSize);
    cudaMalloc((void**)&d_output, imgSize);
    cudaMalloc((void**)&d_filter, filterBytes);
    // copy host memory to device memory
    cudaMemcpy(d_input, imgData, imgSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_filter, filterData, filterBytes, cudaMemcpyHostToDevice);
    // determine GPU device kernel execution configuration
    // launch kernel on device
    // wait for kernel execution to complete, check for errors
    // retrieve results from device
    // use/verify results on host
}