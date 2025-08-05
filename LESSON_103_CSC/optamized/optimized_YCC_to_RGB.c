// optimized_YCC_to_RGB_neon.c
#include <arm_neon.h>
#include <stdint.h>
#include <stdio.h>
#include "optimized_global.h"

// vectorized upsample of one 2×2 chroma quad → 4‑lane vector
static inline int16x4_t upsample_neon_quad(
    uint8_t a, uint8_t b, uint8_t c, uint8_t d
) {
    int16_t tmp[4] = {
        a,
        (a + b) >> 1,
        (a + c) >> 1,
        (a + b + c + d) >> 2
    };
    return vld1_s16(tmp);
}

static void convert_2x2_YCC_block_neon(
    int row, int col,
    const uint8_t Y [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t Cb_ds[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    const uint8_t Cr_ds[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    uint8_t R [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t G [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t B [IMAGE_ROW_SIZE][IMAGE_COL_SIZE]
) {
    // Load and subtract bias from Y
    int16_t y_arr[4] = {
        (int16_t)Y[row  ][col]   - 16,
        (int16_t)Y[row  ][col+1] - 16,
        (int16_t)Y[row+1][col]   - 16,
        (int16_t)Y[row+1][col+1] - 16
    };
    int16x4_t yv = vld1_s16(y_arr);

    // Upsample chroma
    int16x4_t cbv = upsample_neon_quad(
       Cb_ds[(row >> 1)][(col >> 1)],
       Cb_ds[(row >> 1)][(col >> 1) + 1],
       Cb_ds[(row >> 1) + 1][(col >> 1)],
       Cb_ds[(row >> 1) + 1][(col >> 1) + 1]

    );
    int16x4_t crv = upsample_neon_quad(
        Cr_ds[(row>>1)][(col>>1)],
        Cr_ds[(row>>1)][(col>>1) + 1],
        Cr_ds[(row>>1) + 1][(col>>1)],
        Cr_ds[(row>>1) + 1][(col>>1) + 1]
    );

    // Bias chroma by -128
    cbv = vsub_s16(cbv, vdup_n_s16(128));
    crv = vsub_s16(crv, vdup_n_s16(128));

    // Widen all to 32-bit
    int32x4_t y32  = vmovl_s16(yv);
    int32x4_t cb32 = vmovl_s16(cbv);
    int32x4_t cr32 = vmovl_s16(crv);

    // Round before shift
    const int32x4_t round = vdupq_n_s32(1 << (K - 1));

    // Red: R = (D1*y + D2*cr + 0.5) >> K
    int32x4_t r32 = vmulq_n_s32(y32, D1);
    r32 = vqaddq_s32(r32, vmulq_n_s32(cr32, D2));  // saturating 
    r32 = vqaddq_s32(r32, round);
    r32 = vshrq_n_s32(r32, K);

    // Green: G = (D1*y - D3*cr - D4*cb + 0.5) >> K
    int32x4_t g32 = vmulq_n_s32(y32, D1);
    g32 = vqsubq_s32(g32, vmulq_n_s32(cr32, D3));  // saturating subtract
    g32 = vqsubq_s32(g32, vmulq_n_s32(cb32, D4));
    g32 = vqaddq_s32(g32, round);
    g32 = vshrq_n_s32(g32, K);

    // Blue: B = (D1*y + D5*cb + 0.5) >> K
    int32x4_t b32 = vmulq_n_s32(y32, D1);
    b32 = vqaddq_s32(b32, vmulq_n_s32(cb32, D5));
    b32 = vqaddq_s32(b32, round);
    b32 = vshrq_n_s32(b32, K);

    uint16x4_t r_u16 = vqmovun_s32(r32); // saturates to 0–255
    uint16x4_t g_u16 = vqmovun_s32(g32);
    uint16x4_t b_u16 = vqmovun_s32(b32);

    // Store to output
    R[row  ][col]   = vget_lane_u16(r_u16, 0); // red lane 0
    R[row  ][col+1] = vget_lane_u16(r_u16, 1); // red lane 1
    R[row+1][col]   = vget_lane_u16(r_u16, 2); // red lane 2
    R[row+1][col+1] = vget_lane_u16(r_u16, 3); // red lane 3

    G[row  ][col]   = vget_lane_u16(g_u16, 0);
    G[row  ][col+1] = vget_lane_u16(g_u16, 1);
    G[row+1][col]   = vget_lane_u16(g_u16, 2);
    G[row+1][col+1] = vget_lane_u16(g_u16, 3);

    B[row  ][col]   = vget_lane_u16(b_u16, 0);
    B[row  ][col+1] = vget_lane_u16(b_u16, 1);
    B[row+1][col]   = vget_lane_u16(b_u16, 2);
    B[row+1][col+1] = vget_lane_u16(b_u16, 3);

}

void optimized_YCC_to_RGB(
    const uint8_t Y [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t Cb[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    const uint8_t Cr[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    uint8_t R [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t G [IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t B [IMAGE_ROW_SIZE][IMAGE_COL_SIZE]
) {
    for (int r = 0; r < IMAGE_ROW_SIZE; r += 2) {
        for (int c = 0; c < IMAGE_COL_SIZE; c += 2) {
            convert_2x2_YCC_block_neon(r, c, Y, Cb, Cr, R, G, B);
        }
    }
}
