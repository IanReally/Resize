//header files
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    /*

    - Section 1 -
    Initial setup and error checking

    */

    //accept exactly one commandline arguement (the name of the image file to be recovered), else print error and return 1
    if (argc != 2)
    {
        fprintf(stderr, "Filename invalid.\n");
        return 1;
    }
    //assign argv[1] name of "infile", open file
    char *infile = argv[1];
    FILE *rawfile = fopen(infile, "r");

    //if the image cannot be opened for reading, print error with fprintf(to stderr) and return 2
    if (rawfile == NULL)
    {
        fprintf(stderr, "Could not open file.\n");
        return 2;
    }

    /*

    -Section 2-
    Body of program

    */
    //establish datatype "BYTE"
    typedef uint8_t BYTE;
    //establish variable for buffer
    BYTE buffer[512];
    //establish int variable to count jpegs to be output by program
    int jpegcount = -1;
    //establish string for output file names
    char filename[8]; //8 becuase "[X][X][X][.][J][P][G][\0]"
    int eof;

    //while there is still an entire block to read (sizeof(buffer) or 512 BYTES)
    while ((fread(buffer, sizeof(buffer), 1, rawfile)) == 1)
    {
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0)
        {
            //open new outfile, give appropriate name
            jpegcount ++;
            sprintf(filename, "%03i.jpg", jpegcount);
            FILE *outfile = fopen(filename, "w");

            if (outfile == NULL)
            {
                fprintf(stderr, "No file to open");
                fclose(outfile);
                return 3;
            }
            //write blocks of buffer...
            do
            {
                fwrite(buffer, sizeof(buffer), 1, outfile);
                eof = fread(buffer, sizeof(buffer), 1, rawfile);
                //if end of file, close
                if (eof < 1)
                {
                    fclose(rawfile);
                    return 0;
                }

            }
            //...while buffer is not detecting begining of new jpeg
            while (buffer[0] != 0xff || buffer[1] != 0xd8 || buffer[2] != 0xff || (buffer[3] & 0xf0) != 0xe0);

            //new file has been detected, so close current outfile...
            fclose(outfile);
            //seek back to begining of last block read (the block that signified the start of the next jpeg)
            fseek(rawfile, -sizeof(buffer), SEEK_CUR);
        }
    }
    //end program
    fclose(rawfile);
    return 0;
}
