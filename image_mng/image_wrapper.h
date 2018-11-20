/************************************************
*   Author:  Fanchenxin
*   Date:  2018/05/17
*************************************************/
#ifndef __IMAGE_WRAPPER_H__
#define __IMAGE_WRAPPER_H__
#include "log.h"
#include "common.h"

#define IMAGE_LOG_ERR(msg, ...)	    LOG_ERR(LOG_COL_RED_YLW "[IMAGE_ERR]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define IMAGE_LOG_WARNING(msg, ...)	LOG_WARNING(LOG_COL_RED_WHI "[IMAGE_WARNING]"msg LOG_COL_END"\n", ##__VA_ARGS__)
#define IMAGE_LOG_NOTIFY(msg, ...)	LOG_NOTIFY(LOG_COL_NONE "[IMAGE_NOTIFY]"msg LOG_COL_NONE"\n", ##__VA_ARGS__)
#define IMAGE_LOG_NONE
/********************************* BMP header ******************************/
#define BF_TYPE 0x4D42             /* "MB" */

#pragma pack(1)
/**** BMP file header structure ****/
typedef struct {
    uInt16 bfType;           /* Magic number for file */
    uInt32 bfSize;           /* Size of file */
    uInt16 bfReserved1;      /* Reserved */
    uInt16 bfReserved2;      /* ... */
    uInt32 bfOffBits;        /* Offset to bitmap data */
} stBmpFileHeader;

/**** BMP file info structure ****/
typedef struct
{
    uInt32  biSize;           /* Size of info header */
    Int32   biWidth;          /* Width of image */
    Int32   biHeight;         /* Height of image */
    uInt16  biPlanes;         /* Number of color planes */
    uInt16  biBitCount;       /* Number of bits per pixel */
    uInt32  biCompression;    /* Type of compression to use */
    uInt32  biSizeImage;      /* Size of image data */
    Int32   biXPelsPerMeter;  /* X pixels per meter */
    Int32   biYPelsPerMeter;  /* Y pixels per meter */
    uInt32  biClrUsed;        /* Number of colors used */
    uInt32  biClrImportant;   /* Number of important colors */
} stBmpInfoHeader;

/*
 * Constants for the biCompression field...
 */

#define BIT_RGB       0             /* No compression - straight BGR data */
#define BIT_RLE8      1             /* 8-bit run-length compression */
#define BIT_RLE4      2             /* 4-bit run-length compression */
#define BIT_BITFIELDS 3             /* RGB bitmap with RGB masks */

/**** Colormap entry structure ****/
typedef struct
{
    uInt8   rgbBlue;          /* Blue value */
    uInt8   rgbGreen;         /* Green value */
    uInt8   rgbRed;           /* Red value */
    uInt8   rgbReserved;      /* Reserved */
} stRGB;

/**** Bitmap information structure ****/
typedef struct
{
    stBmpInfoHeader   bmiHeader;      /* Image header */
    union {
    	stRGB         bmiColors[256];  /* Image colormap */
    	uInt32        mask[3];        /* RGB masks */
    };
} stBmpInfo;
#pragma pack()

typedef struct
{
	uInt8 magic[3];  /* VIV */
	uInt8 version;
}stVimgFileHeader;

typedef struct
{
	uInt32 format;
	uInt32 tiling;
	uInt32 imageStride;
	uInt32 imageWidth;
	uInt32 imageHeight;
	uInt32 bitsOffset;
}stVimgInfo;

enBOOL isBigEndian();
uInt32 ImageRandGetRGB();
uInt8 * ImageLoadBmp(const char *fileName, stBmpInfo *pBmpInfo);
Int32 ImageSaveBmp(const char *fileName, stBmpInfo *pBmpInfo, uInt8 *bits);
uInt8 * ImageLoadVimg(const char *fileName, stVimgInfo *pVimgInfo);
#endif