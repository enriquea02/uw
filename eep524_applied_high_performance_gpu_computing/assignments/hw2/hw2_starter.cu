
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <profileapi.h> // On the AWS WorkSpace VDE, the windows high-performance counter api
                        // header file is located in 
                        // 'C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um'

#define STB_IMAGE_IMPLEMENTATION // this is needed
#include "util/stb_image.h"  // download from class website files
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "util/stb_image_write.h"  // download from class website files

// #include your error-check macro header file here

// global gaussian blur filter coefficients array here
#define BLUR_FILTER_WIDTH 9  // 9x9 (square) Gaussian blur filter
const float BLUR_FILT[81] = { 0.1084,0.1762,0.2494,0.3071,0.3292,0.3071,0.2494,0.1762,0.1084,0.1762,0.2865,0.4054,0.4994,0.5353,0.4994,0.4054,0.2865,0.1762,0.2494,0.4054,0.5738,0.7066,0.7575,0.7066,0.5738,0.4054,0.2494,0.3071,0.4994,0.7066,0.8703,0.9329,0.8703,0.7066,0.4994,0.3071,0.3292,0.5353,0.7575,0.9329,1.0000,0.9329,0.7575,0.5353,0.3292,0.3071,0.4994,0.7066,0.8703,0.9329,0.8703,0.7066,0.4994,0.3071,0.2494,0.4054,0.5738,0.7066,0.7575,0.7066,0.5738,0.4054,0.2494,0.1762,0.2865,0.4054,0.4994,0.5353,0.4994,0.4054,0.2865,0.1762,0.1084,0.1762,0.2494,0.3071,0.3292,0.3071,0.2494,0.1762,0.1084};

// DEFINE your CUDA blur kernel function(s) here
// blur kernel #1 - global memory only
__global__
void blurKernel(
    // Port List
    // Image Matrix
    unsigned char * in,
    unsigned char * out,
    int w, int h, 
    int BLUR_SIZE, 
    // Blur Matrix
    const float* blurFilter
) {
    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    if (Col < w && Row < h) {
        int pixVal = 0;
        int blurWidth = (2 * BLUR_SIZE) + 1;
        float blurCoefficient = 0.0f;
        float weightedBlurSum = 0.0f;
        // Get the average of the surrounding 2xBLUR_SIZE+1 x 2xBLUR_SIZE+1 box
        for(int blurRow = -BLUR_SIZE; blurRow < (BLUR_SIZE+1); ++blurRow) {
            for(int blurCol = -BLUR_SIZE; blurCol < BLUR_SIZE+1; ++blurCol) {
                int curRow = Row + blurRow;
                int curCol = Col + blurCol;
                // Verify we have a valid image pixel
                if (curRow > -1 && curRow < h && curCol > -1 && curCol < w) {
                    // Pull gaussian filter value
                    blurCoefficient = blurFilter[(blurRow + BLUR_SIZE) * blurWidth + (blurCol + BLUR_SIZE)];
                    weightedBlurSum += blurCoefficient; 
                    // gaussian blur input pixel and round result to the nearest INT value
                    pixVal += (int)roundf(blurCoefficient * in[curRow * w + curCol]);                }
            }
        }
        // Write new pixel value out
        out[Row * w + Col] = (unsigned char)(pixVal / weightedBlurSum);
    }
}


// blur kernel #2 - device shared memory (static alloc)
__global__
void blurKernelStatic(
    // Port List
    // Image Matrix
    unsigned char * in,
    unsigned char * out,
    int w, int h, 
    int BLUR_SIZE, 
    // Blur Matrix
    const float* blurFilter
) {
    __shared__ float s_filter[BLUR_FILTER_WIDTH][BLUR_FILTER_WIDTH];

    for (int i = (threadIdx.y * blockDim.x + threadIdx.x); i < (BLUR_FILTER_WIDTH * BLUR_FILTER_WIDTH); i += (blockDim.x * blockDim.y)) {
        s_filter[i / BLUR_FILTER_WIDTH][i % BLUR_FILTER_WIDTH] = blurFilter[i];
    }

    __syncthreads();
    
    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    if (Col < w && Row < h) {
        int pixVal = 0;
        int blurWidth = (2 * BLUR_SIZE) + 1;
        float blurCoefficient = 0.0f;
        float weightedBlurSum = 0.0f;
        // Get the average of the surrounding 2xBLUR_SIZE+1 x 2xBLUR_SIZE+1 box
        for(int blurRow = -BLUR_SIZE; blurRow < (BLUR_SIZE+1); ++blurRow) {
            for(int blurCol = -BLUR_SIZE; blurCol < BLUR_SIZE+1; ++blurCol) {
                int curRow = Row + blurRow;
                int curCol = Col + blurCol;
                // Verify we have a valid image pixel
                if (curRow > -1 && curRow < h && curCol > -1 && curCol < w) {
                    // Pull gaussian filter value
                    blurCoefficient = s_filter[(blurRow + BLUR_SIZE)][(blurCol + BLUR_SIZE)];
                    weightedBlurSum += blurCoefficient; 
                    // gaussian blur input pixel and round result to the nearest INT value
                    pixVal += (int)roundf(blurCoefficient * in[curRow * w + curCol]);                }
            }
        }
        // Write new pixel value out
        out[Row * w + Col] = (unsigned char)(pixVal / weightedBlurSum);
    }
}


// blur kernel #2 - device shared memory (dynamic alloc)
extern __shared__ float s_filter[];

__global__ void blurKernelDynamic(
    // Port List
    // Image Matrix
    unsigned char * in,
    unsigned char * out,
    int w, int h, 
    int BLUR_SIZE, 
    // Blur Matrix
    const float* blurFilter
) {

    for (int i = (threadIdx.y * blockDim.x + threadIdx.x); i < (BLUR_FILTER_WIDTH * BLUR_FILTER_WIDTH); i += (blockDim.x * blockDim.y)) {
        s_filter[i] = blurFilter[i];
    }

    __syncthreads();

    int Col = blockIdx.x * blockDim.x + threadIdx.x;
    int Row = blockIdx.y * blockDim.y + threadIdx.y;
    if (Col < w && Row < h) {
        int pixVal = 0;
        int blurWidth = (2 * BLUR_SIZE) + 1;
        float blurCoefficient = 0.0f;
        float weightedBlurSum = 0.0f;
        // Get the average of the surrounding 2xBLUR_SIZE+1 x 2xBLUR_SIZE+1 box
        for(int blurRow = -BLUR_SIZE; blurRow < (BLUR_SIZE+1); ++blurRow) {
            for(int blurCol = -BLUR_SIZE; blurCol < BLUR_SIZE+1; ++blurCol) {
                int curRow = Row + blurRow;
                int curCol = Col + blurCol;
                // Verify we have a valid image pixel
                if (curRow > -1 && curRow < h && curCol > -1 && curCol < w) {
                    // Pull gaussian filter value
                    blurCoefficient = s_filter[(blurRow + BLUR_SIZE) * blurWidth + (blurCol + BLUR_SIZE)];
                    weightedBlurSum += blurCoefficient; 
                    // gaussian blur input pixel and round result to the nearest INT value
                    pixVal += (int)roundf(blurCoefficient * in[curRow * w + curCol]);                }
            }
        }
        // Write new pixel value out
        out[Row * w + Col] = (unsigned char)(pixVal / weightedBlurSum);
    }
}


// EXTRA CREDIT
// define host sequential blur-kernel routine


int main()
{
    // Variables
    // Timing variables
    LARGE_INTEGER frequency;
    LARGE_INTEGER start_timer1, end_timer1;
    LARGE_INTEGER start_timer2, end_timer2;
    double elapsed1 = 0.0;
    double elapsed2 = 0.0;

    const char imgFileOut[] = "images/hw2_outimage1.png";

     // read input image from file - be aware of image pixel bit-depth and resolution (horiz x vertical)
    const char filename[] = "images/hw2_testimage1.png";
    int x_cols = 0;
    int y_rows = 0;
    int n_pixdepth = 0;
    unsigned char* imgData = stbi_load(filename, &x_cols, &y_rows, &n_pixdepth, 1);

    printf("Image: %s\n", filename);
    printf("Width: %d pixels\n", x_cols);
    printf("Height: %d pixels\n", y_rows);
    printf("Source channels: %d\n", n_pixdepth);

    int imgSize = x_cols * y_rows * sizeof(unsigned char);

    // setup additional host variables, allocate host memory as needed
    unsigned char* h_imgOut = (unsigned char*)malloc(imgSize);
    unsigned char* d_input = nullptr;
    unsigned char* d_output = nullptr;
    float* d_filter = nullptr;

    cudaError_t cudaStatus = cudaSuccess;

    int radius = BLUR_FILTER_WIDTH / 2;
    size_t filterBytes = sizeof(BLUR_FILT);

    dim3 block(16, 16);
    dim3 grid((x_cols + block.x - 1) / block.x,
            (y_rows + block.y - 1) / block.y);


    QueryPerformanceFrequency(&frequency);

    // START timer #1
    QueryPerformanceCounter(&start_timer1);

    // allocate device memory
    cudaMalloc((void**)&d_input, imgSize);
    cudaMalloc((void**)&d_output, imgSize);
    cudaMalloc((void**)&d_filter, filterBytes);

    // copy host data to device
    cudaMemcpy(d_input, imgData, imgSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_filter, BLUR_FILT, filterBytes, cudaMemcpyHostToDevice);

    // START timer #2
    QueryPerformanceCounter(&start_timer2);
    // launch kernel --- use appropriate heuristics to determine #threads/block and #blocks/grid to ensure coverage of your 2D data range
    blurKernel<<<grid, block>>>(d_input, d_output, x_cols, y_rows, radius, d_filter);
    // blurKernelStatic<<<grid, block>>>(d_input, d_output, x_cols, y_rows, radius, d_filter);
    // blurKernelDynamic<<<grid, block, filterBytes>>>(d_input, d_output, x_cols, y_rows, radius, d_filter);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }

    // call cudaDeviceSynchronize() to wait for the kernel to finish, and return
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();

    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // STOP timer #2
    QueryPerformanceCounter(&end_timer2);
    elapsed2 = (double)(end_timer2.QuadPart - start_timer2.QuadPart) / frequency.QuadPart;

    // retrieve result data from device back to host
    cudaStatus = cudaMemcpy(h_imgOut, d_output, imgSize, cudaMemcpyDeviceToHost);

    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }

    // STOP timer #1
    QueryPerformanceCounter(&end_timer1);
    elapsed1 = (double)(end_timer1.QuadPart - start_timer1.QuadPart) / frequency.QuadPart;
    printf("Kernel execution time: %f seconds\n", elapsed2);
    printf("Total execution time: %f seconds\n", elapsed1);

    // save result output image data to file
    stbi_write_png(imgFileOut, x_cols, y_rows, 1, h_imgOut, x_cols * n_pixdepth);

    // EXTRA CREDIT:
    // start timer #3
    // run host sequential blur routine
    // stop timer #3

    // retrieve and save timer results (write to console or file)
 

Error:
    // free host and device memory
    stbi_image_free(imgData);
    free(h_imgOut);
    cudaFree(d_input);
    cudaFree(d_output);
    cudaFree(d_filter);

    // cudaDeviceReset( ) must be called in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaDeviceReset();

    return 0;
}


