/*

 SENG 440 Project
 Summer 2025
 
*/

#define SIZE_OFFSET 2
#define DATA_OFFSET 54
#define WIDTH_OFFSET 18

/*
* Structure to represent RGB pixel values. Note: BMP files store them in the order b,g,r
*/
typedef struct {
	unsigned char b;
	unsigned char g;
	unsigned char r;  

} Pixel;

/*
* Structure to represent an image
*/
typedef struct {
	int width;
	int height;
	int numPixels;
	Pixel* pixels;
} Image;

/*
* Reads pixel data from an image file
*/
Image readImage(char*);

/*
* Write pixel data to an image file
*/
void writeImage(char*, Image);