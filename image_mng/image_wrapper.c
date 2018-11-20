#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "image_wrapper.h"

static uInt8 getNextColor(enBOOL *up, uInt8 cur, uInt32 mod)
{
	uInt8 next;

	next = cur + (*up ? 1 : -1) * (rand() % mod);
	if ((*up && next < cur) || (!*up && next > cur)) {
		*up = !*up;
		next = cur;
	}

	return next;
}

enBOOL isBigEndian()
{
	typedef union {
		Int16 s;
		Int8 a;
	}uT;
	uT t = {0};
	t.s = 0x0001;
	return (t.a == 0);
}

uInt32 ImageRandGetRGB()
{
	static uInt8 r, g, b;
	static enBOOL r_up, g_up, b_up;
	static enBOOL first = FALSE;

	if(first == FALSE){
		srand(time(NULL));
		r = rand() % 0xff;
		g = rand() % 0xff;
		b = rand() % 0xff;
		r_up = g_up = b_up = TRUE;
		first = TRUE;
	}

	r = getNextColor(&r_up, r, 20);
	g = getNextColor(&g_up, g, 10);
	b = getNextColor(&b_up, b, 5);

	return COLOR_ARGB8(0xFF, r, g, b);
}

uInt8 * ImageLoadBmp(const char *fileName, stBmpInfo *pBmpInfo)
{
    FILE    *fp = NULL;          /* Open file pointer */
    uInt8   *bits = NULL;        /* Bitmap pixel bits */
    uInt32   bitsize;      /* Size of bitmap */
    uInt32   filesize = 0;
    Int32    infosize;     /* Size of header information */
    stBmpFileHeader  header = {0};       /* File header */

    /* Try opening the file; use "rb" mode to read this *binary* file. */
    fp = fopen(fileName, "rb");
    if(!fp){
        /* Failed to open the file. */
        IMAGE_LOG_ERR("Failed to open the file %s", fileName);
        return (NULL);
    }

    /* Read the file header and any following bitmap information... */
	fread(&header, sizeof(stBmpFileHeader), 1, fp);
	IMAGE_LOG_NOTIFY("bfType: %x, bfSize: %d, bfOffBits: %d", header.bfType, header.bfSize, header.bfOffBits);
    if (header.bfType != BF_TYPE){
        /* Not a bitmap file - return NULL... */
        IMAGE_LOG_ERR("*ERROR*  Not a bitmap file, bfType = %x", header.bfType);
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - 14; // sizeof(header);
	/* read bmp info head data */
	fread(&pBmpInfo->bmiHeader, sizeof(stBmpInfoHeader), 1, fp);

    if (infosize > 40){
		/* read color map table */
        Int32 n = (infosize - 40) / 4;
        uInt32 *p = (uInt32 *)(pBmpInfo->bmiColors);
		fread(pBmpInfo->bmiColors, sizeof(uInt32), n, fp);
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp); // ftell return the offset from the head of file

    /* Seek to the image. */
    if (fseek(fp, header.bfOffBits, SEEK_SET) != 0){
        fclose(fp);
        IMAGE_LOG_ERR("*ERROR* bitmap file error");
        return (NULL);
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if ((bitsize = pBmpInfo->bmiHeader.biSizeImage) == 0){
        bitsize = (pBmpInfo->bmiHeader.biWidth) *
                   ((pBmpInfo->bmiHeader.biBitCount + 7) / 8) * abs(pBmpInfo->bmiHeader.biHeight);
    }
    else{
        if ((Int32)bitsize < (COMM_ALIGN((pBmpInfo->bmiHeader.biWidth) *
                   (pBmpInfo->bmiHeader.biBitCount), 8) >> 3) * abs(pBmpInfo->bmiHeader.biHeight)){
            IMAGE_LOG_ERR("*ERROR* bitmap format wrong!");
			fclose(fp);
            return (NULL);
        }
    }

    if (header.bfOffBits + bitsize > filesize){
        IMAGE_LOG_ERR( "*ERROR* bitmap format wrong!");
		fclose(fp);
        return (NULL);
    }

	IMAGE_LOG_NOTIFY("BIT_SIZE =%d", bitsize);
    if ((bits = (unsigned char *)malloc(bitsize)) == NULL){
        /* Couldn't allocate memory - return NULL! */
        fclose(fp);
        IMAGE_LOG_ERR("*ERROR* out-of-memory2");
        return (NULL);
    }

	IMAGE_LOG_NOTIFY("bmp data size = %d", bitsize);

    if (fread(bits, 1, bitsize, fp) < bitsize){
        /* Couldn't read bitmap - free memory and return NULL! */
        free(bits);
        fclose(fp);
        IMAGE_LOG_ERR("*ERROR* read bmp file error");
        return (NULL);
    }
    
    /* OK, everything went fine - return the allocated bitmap... */
    fclose(fp);
    return (bits);
}

Int32 ImageSaveBmp(const char *fileName, stBmpInfo *pBmpInfo, uInt8 *bits)
{
    FILE *fp = NULL;                      /* Open file pointer */
    uInt32    size,                     /* Size of file */
              infosize,                 /* Size of bitmap info */
              bitsize;                  /* Size of bitmap pixels */
    Int32 i = 0;
    uInt32 bmpStride = pBmpInfo->bmiHeader.biWidth * ((pBmpInfo->bmiHeader.biBitCount + 7) / 8);

	fp = fopen(fileName, "wb");
    if (!fp){
        /* Failed to open the file. */
        IMAGE_LOG_ERR("*ERROR*  Failed to open the file %s", fileName);
        return -1;
    }

    /* Figure out the bitmap size */
    if (pBmpInfo->bmiHeader.biSizeImage == 0){
        bitsize = pBmpInfo->bmiHeader.biWidth *
               ((pBmpInfo->bmiHeader.biBitCount + 7) / 8) *
               abs(pBmpInfo->bmiHeader.biHeight);
    }
    else{
        bitsize = pBmpInfo->bmiHeader.biSizeImage;
    }

    /* Figure out the header size */
    infosize = sizeof(stBmpInfoHeader);
    switch (pBmpInfo->bmiHeader.biCompression)
    {
    case BIT_BITFIELDS :
        infosize += 12; /* Add 3 RGB doubleword masks */
        if (pBmpInfo->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RGB :
        if (pBmpInfo->bmiHeader.biBitCount > 8 &&
        pBmpInfo->bmiHeader.biClrUsed == 0)
        break;
    case BIT_RLE8 :
    case BIT_RLE4 :
        if (pBmpInfo->bmiHeader.biClrUsed == 0){
            infosize += (1 << pBmpInfo->bmiHeader.biBitCount) * 4;
        }else{
            infosize += pBmpInfo->bmiHeader.biClrUsed * 4;
        }
        break;
    }

    size = sizeof(stBmpFileHeader) + infosize + bitsize;

    /* Write the file header, bitmap information, and bitmap pixel data... */
	stBmpFileHeader fileHeader;
	fileHeader.bfType = BF_TYPE;
	fileHeader.bfSize = size;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = 14 + infosize;
	fwrite(&fileHeader, sizeof(stBmpFileHeader), 1, fp);
	fwrite(&pBmpInfo->bmiHeader, sizeof(stBmpInfoHeader), 1, fp);

    if (infosize > 40){
        Int32 n = (infosize - 40) / 4;
        uInt32 *p = (uInt32 *)(pBmpInfo->bmiColors);

        while (n > 0){
			fwrite(p, sizeof(uInt32), 1, fp);
            p++;
            n--;
        }
    }

    if((infosize + 14) != ftell(fp)){
		IMAGE_LOG_ERR("ftell = %d, infosize = %d", ftell(fp), infosize);
		return -1;
    }

    for (i = 0; i < abs(pBmpInfo->bmiHeader.biHeight); i++){
        if (pBmpInfo->bmiHeader.biHeight > 0){
            fwrite(bits + bmpStride * i, 1, bmpStride, fp);
        }
        else{
            fwrite(bits + bmpStride * (abs(pBmpInfo->bmiHeader.biHeight) - 1 - i), 1, bmpStride, fp);
        }
    }

    /* OK, everything went fine - return... */
    fclose(fp);
    return (0);
}


uInt8 * ImageLoadVimg(const char *fileName, stVimgInfo *pVimgInfo)
{
	FILE *fp = NULL;
	uInt32 filesize = 0;
	uInt32 bitsize = 0;
	stVimgFileHeader header;
	uInt8 *bits = NULL;
	
	/* Try opening the file; use "rb" mode to read this *binary* file. */
    fp = fopen(fileName, "rb");
    if(!fp){
        /* Failed to open the file. */
        IMAGE_LOG_ERR("Failed to open the file %s", fileName);
        return (NULL);
    }

    /* Read the file header and any following bitmap information... */
	fread(&header, sizeof(stVimgFileHeader), 1, fp);
	if(header.version == 1){
		fread(pVimgInfo, sizeof(stVimgInfo), 1, fp);
	}
	else{
		IMAGE_LOG_ERR("vimg version %d is not support!", header.version);
		goto FAIL;
	}

	fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);

	if(pVimgInfo->imageHeight * pVimgInfo->imageStride + pVimgInfo->bitsOffset > filesize){
		IMAGE_LOG_ERR("Invalid vimg data!");
		goto FAIL;
	}

	if(pVimgInfo->imageWidth == 0 || pVimgInfo->imageHeight == 0){
		IMAGE_LOG_ERR("Invalid vimg data!");
		goto FAIL;
	}

	/* Seek to the image. */
    if (fseek(fp, pVimgInfo->bitsOffset, SEEK_SET) != 0){
        IMAGE_LOG_ERR("*ERROR* Vimg file error");
        goto FAIL;
    }

	bitsize = filesize - pVimgInfo->bitsOffset;
	bits = (uInt8 *)malloc(bitsize * sizeof(uInt8));
	if(bits){
		fread(bits, bitsize, 1, fp);
	}
	else{
		IMAGE_LOG_ERR("Mallo error!");
		goto FAIL;
	}

	fclose(fp);
	return bits;
FAIL:
	fclose(fp);
	return NULL;
}

