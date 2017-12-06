#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

/*
* Implement a program that resizes BMPs.
*/

int main(int argc, char* argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    int resize = atoi(argv[1]);
    if(resize < 1 || resize > 100)
    {
        printf("Resize must be between 1 and 100\n");
        return 2;
    }
    
    char* infile = argv[2];
    char* outfile = argv[3];

    // open input file 
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 3;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 4;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // store original dimensions
    int originalWidth = abs(bi.biWidth);
    int originalHeight = abs(bi.biHeight);
    //int originalSizeImage = bi.biSizeImage;
    //int originalSize = bf.bfSize;
      
    // change dimensions in BITMAPINFOHEADER
    bi.biWidth *= resize;
    bi.biHeight *= resize;
    
    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int originalPadding =  (4 - (originalWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    bi.biSizeImage = (abs(bi.biWidth) * 3 + padding) * abs(bi.biHeight);
    
    // change dimensions in BITMAPFILEHEADER
    // NOTE: bf.bfOffBits is equivalent to sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) here
    bf.bfSize = bf.bfOffBits + bi.biSizeImage;
    
    //printf("Width: %d, %d\n", originalWidth, bi.biWidth);
    //printf("Height: %d, %d\n", originalHeight, bi.biHeight);
    //printf("Size Image: %d, %d\n", originalSizeImage, bi.biSizeImage);
    //printf("Size: %d, %d\n", originalSize, bf.bfSize);
    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0; i < originalHeight; i++)
    {
        // after writing all a scanline's RGB triples (pixels) 'resize' number of times (see below), 
        // return to start of scanline (using fseek) 'resize' number of times (before writing pixels again)
        for (int l = 0; l < resize; l++)
        {
            // Go to start of scanline in inptr
            // [It's i MULTIPLIED by the image width and padding. So on the first scanline i will be 0 and fseek 
            // will just skip over the headers. On the second scanline fseek will offset from start of the line
            // the whole way to the end, i.e. to the start of the next line, ready to read the pixels. Still not sure 
            // how the i incrreasing to 2, 3 etc will work]
            // [NOTE: bf.bfOffBits is equivalent to (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) here]
            fseek(inptr, bf.bfOffBits + i * (originalPadding + (originalWidth * 3)), SEEK_SET);
            // printf("%d, %d, %d . %d\n", i, padding, originalWidth, i * (originalPadding + (originalWidth * 3)));
            
            // iterate over pixels in scanline
            for (int j = 0; j < originalWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write RGB triple to outfile 'resize' number of times
                for (int k = 0; k < resize; k++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }
            // skip over padding, if any
            // QUESTION: shouldn't it be orginalPadding for the infile, not padding? Don't understand!
            fseek(inptr, padding, SEEK_CUR);

            // then add it back (to demonstrate how)
            for (int k = 0; k < padding; k++)
                fputc(0x00, outptr);
            
        }
    }
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // that's all folks
    return 0;
}
