// Function Description:
//  This kernel acts as a low pass filter and takes a average around BLUR_SIZE, verifies pixel validity
//  and writes out the new pixel value.

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