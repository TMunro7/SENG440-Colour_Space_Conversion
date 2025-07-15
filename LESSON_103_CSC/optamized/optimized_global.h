// optimized_global.h
#ifndef OPTIMIZED_GLOBAL_H
#define OPTIMIZED_GLOBAL_H

#include <stdint.h>

#define IMAGE_ROW_SIZE 64
#define IMAGE_COL_SIZE 48

#define K 8 // fixed-point bit shift
#define C11  66
#define C12 129
#define C13  25
#define C21  38
#define C22  74
#define C23 112
#define C31 112
#define C32  94
#define C33  18

#define D1 298
#define D2 409
#define D3 208
#define D4 100
#define D5 516

void optimized_RGB_to_YCC(
    const uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1]
);

void optimized_YCC_to_RGB(
    const uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    const uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    const uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1],
    uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE],
    uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE]
);

#endif
