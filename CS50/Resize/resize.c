#include <cs50.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
//check for correct number of arguments, else print error
 if (argc != 4)
    {
        fprintf(stderr, "Please include 3 arguments. First, a positive integer less than 100. Second, the name of the BMP to be resized. Third, a name for the new resized BMP that will be created.\n");
        return 1;
    }

    // store filenames
    int multiplier = atoi(argv[1]);
    char *infile = argv[2];
    char *outfile = argv[3];

    //print error if size is invalid
    if (multiplier < 1 || multiplier > 100)
    {
        fprintf(stderr, "First argument must be and positive integer less than or equal to 100");
        return 2;
    }

    // open input file ("r" means read)
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 3;
    }

    // open output file ("w" means write)
    FILE *outptr = fopen(outfile, "w");
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
        return 5;
    }
    //save original height, width, and padding values for later calculations
    long originalBiWidth = bi.biWidth;
    long originalBiHeight = abs(bi.biHeight);
    //establish original padding needed based on formula given in walkthrough
    int originalPadding = (4 - (originalBiWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    //establish new height and width vlaues based on multiplier provided by user
    bi.biWidth = (originalBiWidth * multiplier);
    bi.biHeight = (originalBiHeight * multiplier);
    //establish new padding needed based on formula given in walkthrough
    int newPadding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    //calculate new image size and file size to be written in new HEADERS
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth) + newPadding) * abs(bi.biHeight);
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr); 

    // iterate over each of infile's scanlines (rows)
    for (int i = 0, Height = (originalBiHeight); i < Height; i++)
    {
        //initiate loop to write rows of pixels and padding (multiplier - 1) times
        for (int looprow = 0; looprow < (multiplier - 1); looprow ++)
        {
            // iterate over pixels in scanline (row)
            for (int pixelcount = 0; pixelcount < originalBiWidth; pixelcount++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write RGB triple for multiplier times
                for (int k = 0; k < multiplier; k++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }

            }
            //write padding
            for (int m = 0; m < newPadding; m++)
                {
                    fputc(0x00, outptr);
                }

            fseek(inptr, -(originalBiWidth * sizeof(RGBTRIPLE)), SEEK_CUR);
        }

        //repeat for final line (to reach multiplier times)
        //iterate over pixels in scanline (row) for last time
        for (int pixelcount = 0; pixelcount < originalBiWidth; pixelcount++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple for multiplier times
            for (int k = 0; k < multiplier; k++)
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
            }

            //write padding for last time
            for (int m = 0; m < newPadding; m++)
            {
                fputc(0x00, outptr);
            }

        }

         // skip over padding from infile, if any
        fseek(inptr, originalPadding, SEEK_CUR);

    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
