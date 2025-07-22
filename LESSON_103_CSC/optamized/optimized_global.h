// optimized_global.h
#ifndef OPTIMIZED_GLOBAL_H
#define OPTIMIZED_GLOBAL_H

#include <stdint.h>

#define IMAGE_ROW_SIZE 480
#define IMAGE_COL_SIZE 500

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

// Set to 1 if you want the runtime to be printed to consol, 0 if not.
#define print_runtime 1 
// When set to 1, the following setting will cause the program to output pgm files aside from the regular pgm output file
// These additional files will include pgm files that contain the red, green, and blue values of the input file,
// as well as the Cr, Cb, and Y pgm that are calculated from the RGB data. Enabling this option will give more
// outputs, but will decrease runtime. If this option is disabled, only the output file will be produced.
#define return_all_output_files 1

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
