// optimized_YCC_to_RGB_neon.c
#include <arm_neon.h>
#include <stdint.h>
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
    // load 4 Y values, subtract 16
    int16_t y_arr[4] = {
        (int16_t)Y[row  ][col]   - 16,
        (int16_t)Y[row  ][col+1] - 16,
        (int16_t)Y[row+1][col]   - 16,
        (int16_t)Y[row+1][col+1] - 16
    };
    int16x4_t yv = vld1_s16(y_arr);

    // upsample Cb & Cr
    int16x4_t cbv = upsample_neon_quad(
        Cb_ds[row>>1][col>>1],
        Cb_ds[row>>1][col>>1 + 1],
        Cb_ds[row>>1 + 1][col>>1],
        Cb_ds[row>>1 + 1][col>>1 + 1]
    );
    int16x4_t crv = upsample_neon_quad(
        Cr_ds[row>>1][col>>1],
        Cr_ds[row>>1][col>>1 + 1],
        Cr_ds[row>>1 + 1][col>>1],
        Cr_ds[row>>1 + 1][col>>1 + 1]
    );

    // widen + bias chroma by –128
    int32x4_t y32  = vmovl_s16(yv);
    int32x4_t cb32 = vmovl_s16(vsub_s16(cbv, vdup_n_s16(128)));
    int32x4_t cr32 = vmovl_s16(vsub_s16(crv, vdup_n_s16(128)));

    // R = (D1*y + D2*cr + (1<<(K-1))) >>K
    int32x4_t r32 = vmlaq_n_s32(vmulq_n_s32(y32, D1), cr32, D2);
    r32 = vaddq_s32(r32, vdupq_n_s32(1 << (K - 1)));
    r32 = vshrq_n_s32(r32, K);

    // G = (D1*y - D3*cr - D4*cb + bias)>>K
    int32x4_t g32 = vmulq_n_s32(y32, D1);
    g32 = vmlsq_n_s32(g32, cr32, D3);
    g32 = vmlsq_n_s32(g32, cb32, D4);
    g32 = vaddq_s32(g32, vdupq_n_s32(1 << (K - 1)));
    g32 = vshrq_n_s32(g32, K);

    // B = (D1*y + D5*cb + bias)>>K
    int32x4_t b32 = vmlaq_n_s32(vmulq_n_s32(y32, D1), cb32, D5);
    b32 = vaddq_s32(b32, vdupq_n_s32(1 << (K - 1)));
    b32 = vshrq_n_s32(b32, K);

    // clamp 0–255
    r32 = vmaxq_s32(r32, vdupq_n_s32(0)); r32 = vminq_s32(r32, vdupq_n_s32(255));
    g32 = vmaxq_s32(g32, vdupq_n_s32(0)); g32 = vminq_s32(g32, vdupq_n_s32(255));
    b32 = vmaxq_s32(b32, vdupq_n_s32(0)); b32 = vminq_s32(b32, vdupq_n_s32(255));

    // narrow to 16 bits
    int16x4_t r16 = vmovn_s32(r32);
    int16x4_t g16 = vmovn_s32(g32);
    int16x4_t b16 = vmovn_s32(b32);

    // scatter back
    R[row  ][col]   = vget_lane_s16(r16, 0);
    R[row  ][col+1] = vget_lane_s16(r16, 1);
    R[row+1][col]   = vget_lane_s16(r16, 2);
    R[row+1][col+1] = vget_lane_s16(r16, 3);

    G[row  ][col]   = vget_lane_s16(g16, 0);
    G[row  ][col+1] = vget_lane_s16(g16, 1);
    G[row+1][col]   = vget_lane_s16(g16, 2);
    G[row+1][col+1] = vget_lane_s16(g16, 3);

    B[row  ][col]   = vget_lane_s16(b16, 0);
    B[row  ][col+1] = vget_lane_s16(b16, 1);
    B[row+1][col]   = vget_lane_s16(b16, 2);
    B[row+1][col+1] = vget_lane_s16(b16, 3);
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
