#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "optimized_global.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        // If no input file is specified print this message
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    FILE *input_file = fopen(input_filename, "rb");

    if (!input_file) {
        printf("Cannot open file.\n");
        return 1;
    }

    printf("Opened input file: %s\n", input_filename);

    uint8_t R[IMAGE_ROW_SIZE][IMAGE_COL_SIZE];
    uint8_t G[IMAGE_ROW_SIZE][IMAGE_COL_SIZE];
    uint8_t B[IMAGE_ROW_SIZE][IMAGE_COL_SIZE];

    for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
        for (int col = 0; col < IMAGE_COL_SIZE; col++) {
        // Removed the fputc lines since I don't think storing the R, G and B
        // data into their own files has any use, and takes extra time
            R[row][col] = (uint8_t)fgetc(input_file);
            G[row][col] = (uint8_t)fgetc(input_file);
            B[row][col] = (uint8_t)fgetc(input_file);
        }
    }
    fclose(input_file);

    if (return_all_output_files) {
        // === Write R Channel as PGM ===
        FILE *f_R = fopen("output_R.pgm", "w");
        if (!f_R) {
            fprintf(stderr, "Failed to open output_R.pgm\n");
            return 1;
        }
        fprintf(f_R, "P2\n%d %d\n255\n", IMAGE_COL_SIZE, IMAGE_ROW_SIZE);
        for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
            for (int col = 0; col < IMAGE_COL_SIZE; col++) {
                fprintf(f_R, "%3d ", R[row][col]);
            }
            fprintf(f_R, "\n");
        }
        fclose(f_R);

        // === Write G Channel as PGM ===
        FILE *f_G = fopen("output_G.pgm", "w");
        if (!f_G) {
            fprintf(stderr, "Failed to open output_G.pgm\n");
            return 1;
        }
        fprintf(f_G, "P2\n%d %d\n255\n", IMAGE_COL_SIZE, IMAGE_ROW_SIZE);
        for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
            for (int col = 0; col < IMAGE_COL_SIZE; col++) {
                fprintf(f_G, "%3d ", G[row][col]);
            }
            fprintf(f_G, "\n");
        }
        fclose(f_G);

        // === Write B Channel as PGM ===
        FILE *f_B = fopen("output_B.pgm", "w");
        if (!f_B) {
            fprintf(stderr, "Failed to open output_B.pgm\n");
            return 1;
        }
        fprintf(f_B, "P2\n%d %d\n255\n", IMAGE_COL_SIZE, IMAGE_ROW_SIZE);
        for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
            for (int col = 0; col < IMAGE_COL_SIZE; col++) {
                fprintf(f_B, "%3d ", B[row][col]);
            }
            fprintf(f_B, "\n");
        }
        fclose(f_B);
    }

    uint8_t Y[IMAGE_ROW_SIZE][IMAGE_COL_SIZE];
    uint8_t Cb[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1];
    uint8_t Cr[IMAGE_ROW_SIZE >> 1][IMAGE_COL_SIZE >> 1];

    // Call the conversion function
    optimized_RGB_to_YCC(R, G, B, Y, Cb, Cr);

    if(return_all_output_files){
        // === Write Y as PGM ===
        FILE *f_Y = fopen("output_Y.pgm", "w");
        if (!f_Y) {
            fprintf(stderr, "Failed to write output_Y.pgm\n");
            return 1;
        }
        fprintf(f_Y, "P2\n%d %d\n255\n", IMAGE_COL_SIZE, IMAGE_ROW_SIZE);
        for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
            for (int col = 0; col < IMAGE_COL_SIZE; col++) {
                fprintf(f_Y, "%3d ", Y[row][col]);
            }
            fprintf(f_Y, "\n");
        }
        fclose(f_Y);
    
        // === Write Cb as PGM ===
        FILE *f_Cb = fopen("output_Cb.pgm", "w");
        if (!f_Cb) {
            fprintf(stderr, "Failed to write output_Cb.pgm\n");
            return 1;
        }

        // === Write Cr as PGM ===
        FILE *f_Cr = fopen("output_Cr.pgm", "w");
        if (!f_Cr) {
            fprintf(stderr, "Failed to write output_Cr.pgm\n");
            return 1;
        }


        fprintf(f_Cb, "P2\n%d %d\n255\n", IMAGE_COL_SIZE >> 1, IMAGE_ROW_SIZE >> 1);
        fprintf(f_Cr, "P2\n%d %d\n255\n", IMAGE_COL_SIZE >> 1, IMAGE_ROW_SIZE >> 1);

        for (int row = 0; row < (IMAGE_ROW_SIZE >> 1); row++) {
            for (int col = 0; col < (IMAGE_COL_SIZE >> 1); col++) {
                fprintf(f_Cb, "%3d ", Cb[row][col]);
                fprintf(f_Cr, "%3d ", Cr[row][col]);
            }
            fprintf(f_Cb, "\n");
            fprintf(f_Cr, "\n");
        }
        fclose(f_Cb);
        fclose(f_Cr);
    }
    optimized_YCC_to_RGB(Y, Cb, Cr, R, G, B);

    FILE *f_output = fopen("output_RGB.pgm", "w");
    if (!f_output) {
        fprintf(stderr, "Failed to open output_RGB.pgm\n");
        return 1;
    }
    fprintf(f_output, "P3\n%d %d\n255\n", IMAGE_COL_SIZE, IMAGE_ROW_SIZE);
    for (int row = 0; row < IMAGE_ROW_SIZE; row++) {
        for (int col = 0; col < IMAGE_COL_SIZE; col++) {
            fprintf(f_output, "%3d %3d %3d  ", R[row][col], G[row][col], B[row][col]);
        }
        fprintf(f_output, "\n");
    }
    fclose(f_output);

    return 0;
}