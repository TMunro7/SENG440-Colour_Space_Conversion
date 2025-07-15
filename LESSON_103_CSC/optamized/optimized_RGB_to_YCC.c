#include <stdint.h>
#include "optimized_global.h"

// Downsample 2x2 chroma values by averaging (truncation)
static uint8_t chroma_downsample(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    int temp = a + b + c + d;
  return (uint8_t)(temp >> 2); // truncating average
  // return a;  // use this for downsampling by discarding 
}

// Convert a single 2x2 RGB block to YCC
static void convert_2x2_block(
    int row, int col,
    const uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1]
) {
    int r00 = R[row][col],     r01 = R[row][col + 1];
    int r10 = R[row + 1][col], r11 = R[row + 1][col + 1];

    int g00 = G[row][col],     g01 = G[row][col + 1];
    int g10 = G[row + 1][col], g11 = G[row + 1][col + 1];

    int b00 = B[row][col],     b01 = B[row][col + 1];
    int b10 = B[row + 1][col], b11 = B[row + 1][col + 1];

    // Luminance (Y)
    Y[row][col]       = (uint8_t)(((16 << K) + C11*r00 + C12*g00 + C13*b00) >> K);
    Y[row][col + 1]   = (uint8_t)(((16 << K) + C11*r01 + C12*g01 + C13*b01) >> K);
    Y[row + 1][col]   = (uint8_t)(((16 << K) + C11*r10 + C12*g10 + C13*b10) >> K);
    Y[row + 1][col + 1] = (uint8_t)(((16 << K) + C11*r11 + C12*g11 + C13*b11) >> K);

    // Chrominance Cb
    uint8_t cb00 = (uint8_t)(((128 << K) - C21*r00 - C22*g00 + C23*b00) >> K);
    uint8_t cb01 = (uint8_t)(((128 << K) - C21*r01 - C22*g01 + C23*b01) >> K);
    uint8_t cb10 = (uint8_t)(((128 << K) - C21*r10 - C22*g10 + C23*b10) >> K);
    uint8_t cb11 = (uint8_t)(((128 << K) - C21*r11 - C22*g11 + C23*b11) >> K);

    // Chrominance Cr
    uint8_t cr00 = (uint8_t)(((128 << K) + C31*r00 - C32*g00 - C33*b00) >> K);
    uint8_t cr01 = (uint8_t)(((128 << K) + C31*r01 - C32*g01 - C33*b01) >> K);
    uint8_t cr10 = (uint8_t)(((128 << K) + C31*r10 - C32*g10 - C33*b10) >> K);
    uint8_t cr11 = (uint8_t)(((128 << K) + C31*r11 - C32*g11 - C33*b11) >> K);

    // Store downsampled chroma
    Cb[row >> 1][col >> 1] = chroma_downsample(cb00, cb01, cb10, cb11);
    Cr[row >> 1][col >> 1] = chroma_downsample(cr00, cr01, cr10, cr11);
}

// Top-level function to walk through the whole image
void optimized_RGB_to_YCC(
    const uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1]
) {
    for (int row = 0; row < IMAGE_ROW_SIZE; row += 2) {
        for (int col = 0; col < IMAGE_COL_SIZE; col += 2) {
            convert_2x2_block(row, col, R, G, B, Y, Cb, Cr);
        }
    }
}
