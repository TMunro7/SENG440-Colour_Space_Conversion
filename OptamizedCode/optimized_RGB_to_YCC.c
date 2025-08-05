// optimized_RGB_to_YCC_neon.c
#include <arm_neon.h>
#include <stdint.h>
#include "optimized_global.h"

// scalar-downsample helper (avg of 4 lanes >>2)
static inline uint8_t chroma_downsample_neon(int16x4_t v) {
    int sum = vget_lane_s16(v, 0)
            + vget_lane_s16(v, 1)
            + vget_lane_s16(v, 2)
            + vget_lane_s16(v, 3);
    return (uint8_t)(sum >> 2);
}

static void convert_2x2_block_neon(
    int row, int col,
    const uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Cb[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    uint8_t Cr[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1]
) {
    // load a 2×2 RGB patch into 4‑lane vectors
    int16_t r_vals[4] = {
        R[row  ][col],   R[row  ][col+1],
        R[row+1][col],   R[row+1][col+1]
    };
    int16_t g_vals[4] = {
        G[row  ][col],   G[row  ][col+1],
        G[row+1][col],   G[row+1][col+1]
    };
    int16_t b_vals[4] = {
        B[row  ][col],   B[row  ][col+1],
        B[row+1][col],   B[row+1][col+1]
    };
    int16x4_t rv = vld1_s16(r_vals);
    int16x4_t gv = vld1_s16(g_vals);
    int16x4_t bv = vld1_s16(b_vals);

    // widen to 32-bit
    int32x4_t r32 = vmovl_s16(rv);
    int32x4_t g32 = vmovl_s16(gv);
    int32x4_t b32 = vmovl_s16(bv);

    // Y = (16<<K + C11·R + C12·G + C13·B) >>K, clamp 0–255
    int32x4_t y32 = vdupq_n_s32(16 << K);
    y32 = vmlaq_n_s32(y32, r32, C11);
    y32 = vmlaq_n_s32(y32, g32, C12);
    y32 = vmlaq_n_s32(y32, b32, C13);
    y32 = vshrq_n_s32(y32, K);
    y32 = vmaxq_s32(y32, vdupq_n_s32(0));
    y32 = vminq_s32(y32, vdupq_n_s32(255));
    int16x4_t y16 = vmovn_s32(y32);

    // scatter Y
    Y[row  ][col]   = vget_lane_s16(y16, 0);
    Y[row  ][col+1] = vget_lane_s16(y16, 1);
    Y[row+1][col]   = vget_lane_s16(y16, 2);
    Y[row+1][col+1] = vget_lane_s16(y16, 3);

    // per-pixel Cb
    int32x4_t cb32 = vdupq_n_s32(128 << K);
    cb32 = vmlsq_n_s32(cb32, r32, C21);
    cb32 = vmlsq_n_s32(cb32, g32, C22);
    cb32 = vmlaq_n_s32(cb32, b32, C23);
    cb32 = vshrq_n_s32(cb32, K);
    cb32 = vmaxq_s32(cb32, vdupq_n_s32(0));
    cb32 = vminq_s32(cb32, vdupq_n_s32(255));
    int16x4_t cb16 = vmovn_s32(cb32);

    // per-pixel Cr
    int32x4_t cr32 = vdupq_n_s32(128 << K);
    cr32 = vmlaq_n_s32(cr32, r32, C31);
    cr32 = vmlsq_n_s32(cr32, g32, C32);
    cr32 = vmlsq_n_s32(cr32, b32, C33);
    cr32 = vshrq_n_s32(cr32, K);
    cr32 = vmaxq_s32(cr32, vdupq_n_s32(0));
    cr32 = vminq_s32(cr32, vdupq_n_s32(255));
    int16x4_t cr16 = vmovn_s32(cr32);

    // downsample chroma
    uint8_t cb_ds = chroma_downsample_neon(cb16);
    uint8_t cr_ds = chroma_downsample_neon(cr16);

    Cb[row>>1][col>>1] = cb_ds;
    Cr[row>>1][col>>1] = cr_ds;
}

void optimized_RGB_to_YCC(
    const uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Cb[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1],
    uint8_t Cr[IMAGE_ROW_SIZE>>1][IMAGE_COL_SIZE>>1]
) {
    for (int r = 0; r < IMAGE_ROW_SIZE; r += 2) {
        for (int c = 0; c < IMAGE_COL_SIZE; c += 2) {
            convert_2x2_block_neon(r, c, R, G, B, Y, Cb, Cr);
        }
    }
}
