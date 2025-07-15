/*

 Main file to do conversion between RGB and YCC
*/

#include <stdlib.h>
#include <stdio.h>
#include "ImageIO.h"

#define yRConst 2498396
#define yGConst 4904878
#define yBConst 952566

#define cbRConst 2498407
#define cbGConstA 4904867
#define cbGConstB 1437774
#define cbBConst 7403274

#define crRConst  5857443
#define crGConstA 4904877
#define crGConstB 4272902
#define crBConst  952567

/*
 Transforms an RGB image to YCbCr components. 
*/
void RGBtoYCC(char* filename)
{	
	int i;
	int k;
	int chromaI;

	int rA;
	int gA;
	int bA;
	
	int rB;
	int gB;
	int bB;

	int yi;
	int yk;

	int cbA;
	int cbGreenIntA;
	int cbB;
	int cbGreenIntB;
	int cbGreen;
	int cbBlue;
	
	int crA;
	int crGreenIntA;
	int crB;
	int crGreenIntB;
	int crRed;
	int crGreen;

	int chromaPixelNum;
	int chromaWidth; 

	Image rgb = readImage(filename);
	Image yImage;
	Image cbImage;
	Image crImage;
	
	chromaPixelNum = rgb.numPixels >> 1;
	chromaWidth = rgb.width >> 1;
	
	// Set up Y output image
	yImage.pixels = (Pixel*) malloc(sizeof(Pixel) * rgb.numPixels);
	yImage.numPixels = rgb.numPixels;
	yImage.width = rgb.width;
	yImage.height = rgb.height;

	// Set up Cb output image
	cbImage.pixels = (Pixel*) malloc(sizeof(Pixel) * rgb.numPixels);
	cbImage.numPixels = chromaPixelNum;
	cbImage.width = chromaWidth;
	cbImage.height = rgb.height;
		
	// Set up Cr output image
	crImage.pixels = (Pixel*) malloc(sizeof(Pixel) * rgb.numPixels);
	crImage.numPixels = chromaPixelNum;
	crImage.width = chromaWidth;
	crImage.height = rgb.height;
	
	for(i = 1; i < rgb.numPixels; i += 2)
	{
		k = i + 1;
		chromaI = i >> 1;
			
		// Load pixel values
		rA = rgb.pixels[i].r;
		gA = rgb.pixels[i].g;
		bA = rgb.pixels[i].b;

		rB = rgb.pixels[k].r;
		gB = rgb.pixels[k].g;
		bB = rgb.pixels[k].b;
	
		// Calculate Y values
		yi = ((yRConst * rA + yGConst * gA + yBConst * bA) >> 23) + 16;
		yk = ((yRConst * rB + yGConst * gB + yBConst * bB) >> 23) + 16;

		asm("USAT %0, #8, %1" : "=r" (yi) : "r" (yi));
		asm("USAT %0, #8, %1" : "=r" (yk) : "r" (yk));

		yImage.pixels[i].r = yi;
		yImage.pixels[i].g = yi;
		yImage.pixels[i].b = yi;

		yImage.pixels[k].r = yk;
		yImage.pixels[k].g = yk;
		yImage.pixels[k].b = yk;
		
		// Calculate Cb Values
		cbA = (bA * cbBConst - rA * cbRConst - gA * cbGConstA);
		cbB = (bB * cbBConst - rB * cbRConst - gB * cbGConstA);

		cbA = cbA >> 24;
		cbB = cbB >> 24;

		cbGreenIntA = -(cbA * cbGConstB);
		cbGreenIntB = -(cbB * cbGConstB);

		cbGreenIntA = cbGreenIntA >> 23;
		cbGreenIntB = cbGreenIntB >> 23;

		cbGreen = cbGreenIntA + cbGreenIntB + 128;
		cbBlue = cbA + cbB + 128;

		asm("USAT %0, #8, %1" : "=r" (cbGreen) : "r" (cbGreen));
		asm("USAT %0, #8, %1" : "=r" (cbBlue) : "r" (cbBlue));

		cbImage.pixels[chromaI].r = 0;
		cbImage.pixels[chromaI].g = cbGreen;
		cbImage.pixels[chromaI].b = cbBlue;

		// Calculate Cr values
		crA = (crRConst * rA - crGConstA * gA - crBConst * bA);
		crB = (crRConst * rB - crGConstA * gB - crBConst * bB);

		crA = crA >> 24;
		crB = crB >> 24;
		
		crGreenIntA = -(crA * crGConstB);
		crGreenIntB = -(crB * crGConstB);

		crGreenIntA = crGreenIntA >> 23;
		crGreenIntB = crGreenIntB >> 23;

		crRed = crA + crB + 128;
		crGreen = crGreenIntA + crGreenIntB + 128;

		asm("USAT %0, #8, %1" : "=r" (crRed) : "r" (crRed));
		asm("USAT %0, #8, %1" : "=r" (crGreen) : "r" (crGreen));

		crImage.pixels[chromaI].r = crRed;
		crImage.pixels[chromaI].g = crGreen;
		crImage.pixels[chromaI].b = 0;

	}	
	
	writeImage("outY.bmp", yImage);
	writeImage("outCb.bmp", cbImage);
	writeImage("outCr.bmp", crImage);

	free(rgb.pixels);
}

/*
 Transforms YCbCr components to an RGB Image. 
*/
int YCCtoRGB(char* yfile, char* cbfile, char* crfile)
{
	Image yImage = readImage(yfile);
	Image cbImage = readImage(cbfile);
	Image crImage = readImage(crfile);
	Image rgbImage;

	rgbImage.pixels = (Pixel*) malloc(sizeof(Pixel) * yImage.numPixels);
	rgbImage.width = yImage.width;
	rgbImage.height = yImage.height;
	rgbImage.numPixels = yImage.numPixels;
	
	int i;
	int chromaI;

	int r;
	int g;
	int b;
	
	for(i = 0; i < yImage.numPixels; i++)
	{
		chromaI = i >> 1;
		
		// Calculate RGB values
		r = yImage.pixels[i].r + crImage.pixels[chromaI].r - 144;
		g = yImage.pixels[i].g + cbImage.pixels[chromaI].g + crImage.pixels[chromaI].g - 272;
		b = yImage.pixels[i].b + cbImage.pixels[chromaI].b - 144;

		asm("USAT %0, #8, %1" : "=r" (r) : "r" (r));
		asm("USAT %0, #8, %1" : "=r" (g) : "r" (g));
		asm("USAT %0, #8, %1" : "=r" (b) : "r" (b));

		rgbImage.pixels[i].r = r;
		rgbImage.pixels[i].g = g;
		rgbImage.pixels[i].b = b;
	}
	
	writeImage("outRGB.bmp", rgbImage);
	
	free(yImage.pixels);
	free(cbImage.pixels);
	free(crImage.pixels);

	return 0;
}

int main(int argc, char* argv[])
{
	if(argc == 2)
	{
		printf("RGB to YCC...\n");
		RGBtoYCC(argv[1]);
	}
	else if (argc == 4)
	{
		printf("YCC to RGB...\n");
		YCCtoRGB(argv[1],argv[2],argv[3]);
	}
	else
	{
		printf("Bad argument format.\n");
	}

	return 0;
}
