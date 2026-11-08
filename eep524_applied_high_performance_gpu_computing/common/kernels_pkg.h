    // ---- Function Description:
    //  Kernels Package Header File

    #ifndef CUDA_KERNELS_PACKAGE_H
    #define CUDA_KERNELS_PACKAGE_H
        // ---- Libraries, Includes, etc...
        #include <cuda_runtime.h>

        // ---- Kernel Prototypes
        //  This kernel prints out global thread ID, block ID, and thread ID.
        //  on a single line.
        __global__ void printFKernel (
        );

        //  1D vector manipulation. SAXPY = single-precision Y = (A * X) + Y.
        //  All terms are 32-bit floats. Vectors X and Y must be the same width.
        //  Output is a 32-bit vector.
        __global__ void saxPyKernel(
            int i_vector_width,
            float i_scalarA,
            float *vectorX,
            float *vectorY
        );

        //  2D matrix addition. All terms are 32-bit floats. 
        //  Matrices X and Y must be the same MxN dimensions.
        //  Output is a MxN 32-bit matrix. Kernel does bound 
        //  checking to ensure that threads are valid for the 
        //  given matrix dimensions.
        __global__ void MatrixAdditionKernel(
            // Port List        
            int i_m_rows,
            int i_n_cols,
            float *matrixX,
            float *matrixY,
            float *matrixZ
        );

        //  3D volume grid addition. All terms are 32-bit floats.
        //  Volume grids X and Y must be the same dimensions (M x N x P).
        //  Output is a 3D volume grid of the same dimensions. Kernel does bound
        //  checking to ensure that threads are valid for the
        //  given volume grid dimensions.
        __global__ void volumeGridAdditionKernel(
            // Port List
            int i_m_rows,
            int i_n_cols,
            int i_p_depth,
            float *volumeGridX,
            float *volumeGridY,
            float *volumeGridZ
        );

    #endif /* CUDA_KERNELS_PACKAGE_H */