#include <stdint.h>
#include "optimized_global.h"

// Fixed-point saturation to clamp values between 0 and 255
static uint8_t saturate(int value) {
    if (value > 255) return 255;
    if (value < 0) return 0;
    return (uint8_t)value;
}

// Upsample one 2x2 chroma block into a 2x2 full-resolution block
static void upsample_chroma(
    uint8_t C00_p, uint8_t C01_p, uint8_t C10_p, uint8_t C11_p,
    uint8_t *TL, uint8_t *TR, uint8_t *BL, uint8_t *BR
) {
    // Top-left stays the same
    *TL = C00_p;

    // Top-right: average of C00 and C01 (truncate)
    *TR = (uint8_t)(((int)C00_p + C01_p) >> 1);

    // Bottom-left: average of C00 and C10 (truncate)
    *BL = (uint8_t)(((int)C00_p + C10_p) >> 1);

    // Bottom-right: average of all 4 (truncate)
    *BR = (uint8_t)(((int)C00_p + C01_p + C10_p + C11_p) >> 2);

}

// Convert a single 2x2 YCC block to RGB
static void convert_2x2_YCC_block(
    int row, int col,
    const uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t Cb_ds[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    const uint8_t Cr_ds[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE]
) {
    uint8_t cb00, cb01, cb10, cb11;
    uint8_t cr00, cr01, cr10, cr11;

    // Upsample chroma for this 2x2 region
    upsample_chroma(
        Cb_ds[row >> 1][col >> 1], Cb_ds[row >> 1][(col >> 1) + 1],
        Cb_ds[(row >> 1) + 1][col >> 1], Cb_ds[(row >> 1) + 1][(col >> 1) + 1],
        &cb00, &cb01, &cb10, &cb11
    );

    upsample_chroma(
        Cr_ds[row >> 1][col >> 1], Cr_ds[row >> 1][(col >> 1) + 1],
        Cr_ds[(row >> 1) + 1][col >> 1], Cr_ds[(row >> 1) + 1][(col >> 1) + 1],
        &cr00, &cr01, &cr10, &cr11
    );

    int y, cb, cr;

    // Top-left pixel
    y  = ((int)Y[row + 0][col + 0]) - 16;
    cb = ((int)cb00) - 128;
    cr = ((int)cr00) - 128;
    R[row + 0][col + 0] = saturate((D1 * y + D2 * cr + (1 << (K - 1))) >> K);
    G[row + 0][col + 0] = saturate((D1 * y - D3 * cr - D4 * cb + (1 << (K - 1))) >> K);
    B[row + 0][col + 0] = saturate((D1 * y + D5 * cb + (1 << (K - 1))) >> K);

    // Top-right pixel
    y  = ((int)Y[row + 0][col + 1]) - 16;
    cb = ((int)cb01) - 128;
    cr = ((int)cr01) - 128;
    R[row + 0][col + 1] = saturate((D1 * y + D2 * cr + (1 << (K - 1))) >> K);
    G[row + 0][col + 1] = saturate((D1 * y - D3 * cr - D4 * cb + (1 << (K - 1))) >> K);
    B[row + 0][col + 1] = saturate((D1 * y + D5 * cb + (1 << (K - 1))) >> K);

    // Bottom-left pixel
    y  = ((int)Y[row + 1][col + 0]) - 16;
    cb = ((int)cb10) - 128;
    cr = ((int)cr10) - 128;
    R[row + 1][col + 0] = saturate((D1 * y + D2 * cr + (1 << (K - 1))) >> K);
    G[row + 1][col + 0] = saturate((D1 * y - D3 * cr - D4 * cb + (1 << (K - 1))) >> K);
    B[row + 1][col + 0] = saturate((D1 * y + D5 * cb + (1 << (K - 1))) >> K);

    // Bottom-right pixel
    y  = ((int)Y[row + 1][col + 1]) - 16;
    cb = ((int)cb11) - 128;
    cr = ((int)cr11) - 128;
    R[row + 1][col + 1] = saturate((D1 * y + D2 * cr + (1 << (K - 1))) >> K);
    G[row + 1][col + 1] = saturate((D1 * y - D3 * cr - D4 * cb + (1 << (K - 1))) >> K);
    B[row + 1][col + 1] = saturate((D1 * y + D5 * cb + (1 << (K - 1))) >> K);
}

// Top-level function to process entire image
void optimized_YCC_to_RGB(
    const uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    const uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE]
) {
    for (int row = 0; row < IMAGE_ROW_SIZE; row += 2) {
        for (int col = 0; col < IMAGE_COL_SIZE; col += 2) {
            convert_2x2_YCC_block(row, col, Y, Cb, Cr, R, G, B);
        }
    }
}
