/*****************************************
* recover.c
*
* Computer Science 50
* Problem Set 4
*
* Recovers JPEGs from a forensic image.
******************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cs50.h>

// need an unsigned integer type of 8 bits (1 byte) for array of 512 bytes
typedef uint8_t BYTE;
/** NOTE: An int is 4 bytes long so int buffer[] would be 2048 bytes, not 512 **/

// define 512 as a constant (can't be changed)
#define BLOCKSIZE 512

int main (int argc, char* argv[])
{
    // declare an array to hold 512 bytes from memory card
    BYTE buffer[BLOCKSIZE];

    // declare fileName variable for use in sprintf
    char fileName[10];

    // ensure proper usage
    if (argc != 2)
    {
        printf("Usage: ./recover filename.raw\n");
        return 1;
    }
    // open memory card file
    FILE* memCardFile = fopen(argv[1], "r");
    if (memCardFile == NULL)
    {
        fprintf(stderr, "Could not open file");
        return 2;
    }

    // declare JPEG counter variable
    int JPEGCount = 0;

    // declare file pointer before assigning it fopen (below) to prevent error "use of undeclared identifier 'JPEGfile'"
    FILE* JPEGfile = NULL;

    //repeat until end of memory card
    // while number of elements successfully read in equals 1 (i.e. while a 512B block remains)
    while (fread(buffer, BLOCKSIZE, 1, memCardFile) == 1)
    {
        // NOTE: Don't need to use fread here as it's already run in the while loop condition

        // if 512B block (buffer) contains start of a new JPEG (signature 4 bytes)
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] & 0xf0) == 0xe0)
        {
           // check if JPEG already created
           if (JPEGCount == 0) // if no JPEGs created yet
           {
               // create first JPEG file
               sprintf(fileName, "%03d.jpg", JPEGCount); // create file name
               JPEGfile = fopen(fileName, "w"); // create/open file
               if (JPEGfile == NULL)
                    {
                        fprintf(stderr, "Could not open file");
                        return 3;
                    }
               JPEGCount++;

               // write 512B from buffer to new JPEG file
               fwrite(&buffer, BLOCKSIZE, 1, JPEGfile);

           } else // if one or more JPEGs already created
            // close previous file
            fclose(JPEGfile); // on second loop will close first JPEG file, "000.jpg"

            // then open new one
            sprintf(fileName, "%03i.jpg", JPEGCount); // filename will be "001.jpg" on 2nd loop due to JPEG++
            JPEGfile = fopen(fileName, "w");
            if (JPEGfile == NULL)
                {
                    fprintf(stderr, "Could not open file");
                    return 3;
                }
            JPEGCount++;
            // write 512B from buffer to new JPEG file
            fwrite(&buffer, BLOCKSIZE, 1, JPEGfile);
        }
        // if read 512 bytes in (buffer) and not reached start of new JPEG (signature 4 bytes)
        else
        {
          // check if already found/created a JPEG
          if (JPEGCount == 0) // if a JPEG not found/created yet (i.e. in grey blocks before red block with star)
          {
            // discard (i.e. don't write) those 512 bytes and go back to start of while loop (i.e. read next 512B into buffer)
            continue;
          }
          else // if one or more JPEGs already found/created (e.g. in read blocks between red-star-block and blue-star-block)
          {
            // write 512B from buffer to current JPEG file until beginning of new JPEG found
            fwrite(&buffer, BLOCKSIZE, 1, JPEGfile);
          }
        }

    }
    fclose(JPEGfile);
    fclose(memCardFile);

    return 0;
}

