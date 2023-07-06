/***************************************************************************
Copyright (C) 2021  Skibblebip

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
****************************************************************************/
/***************************************************************************
* File:  main.c
* Author:  SkibbleBip
* Procedures:
* calcFPSDuration       -calculates the duration between each frame by calling
*                               decompressAndDisplay on the first frame and
*                               measuring it's duration of completion
* calcFPSDuration       -decompresses and displays the current frame being
*                               processed
* processFileErrors     -handles any videofile error events that occured when
*                               reading the videofile header
* main                  -main driver function
***************************************************************************/

#include <bits/stdint-uintn.h>
#include <bits/types/struct_timeval.h>
#include <endian.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

#include "../ASCII.h"

#define         clear()                 printf("\033[0;0H");fflush(stdout);

#define			DURATION(START, STOP) \
        ( (STOP.tv_sec - START.tv_sec) * 1000000 + STOP.tv_usec - START.tv_usec )

#define         READ_BUFFER_SIZE        65536


//unsigned long calcFPSDuration(int input, z_stream* s, uint8_t x, uint8_t y);
uint8_t* decompress(int input, z_stream* s, uint16_t array_size);
void decompressAndDisplay(int input, z_stream* s, uint8_t x, uint8_t y);
void display(uint8_t* frame, uint8_t x, uint16_t array_size);
enum Header_Error processFileErrors(ASCIIheader a);
/*function prototypes*/

/***************************************************************************
* int main(int argc, char* args[])
* Author: SkibbleBip
* Date: 08/25/2021
* Description: The main driver function
*
* Parameters:
*        argc   I/P     int     number of inputted arguments
*        args   I/P     char*[] pointer to c-strings of the inputted arguments
*        main   O/P     int     return value of program
**************************************************************************/
int main(int argc, char* args[])
{
        int inputFile           = 0;
        uint8_t x, y, fps       = 0;
        uint16_t frames         = 0;
        unsigned long frameDuration;

        ASCIIheader ah;
        /*ASCII video file header*/
        z_stream strm;
        /*Zlib stream information*/


        if(argc != 2){
        /*check program argument count*/
                char* cpr =
                "ASCII Video Player Version 1, "
                "Copyright (C) 2021 SkibbleBip\n"
                "ASCII Video Player comes with ABSOLUTELY NO WARRANTY; "
                "for details\ntype '-h'. This is free software, and you are "
                "welcome\nto redistribute it under certain conditions.";

                printf("%s\n\n\tUsage: %s [file]\n\n", cpr, args[0]);
                return 0;
        }

        if((inputFile = open(args[1], O_RDONLY)) < 0){
        /*attempt to open the file passed as an argument*/
                printf("Failed to open %s\n", args[1]);
                return 0;
        }

        if(sizeof(ASCIIheader) != read(inputFile, &ah, sizeof(ASCIIheader))){
        /*read in the file header for the video file*/
                fprintf(stderr, "Failed to read input file header\n");
                close(inputFile);
                return 1;
        }

        enum Header_Error ret;
        if( (ret = processFileErrors(ah)) != OK){
        /*for the time being, only 5-bit stuffed compression supported*/
                if(ret == COMPRESSION)
                        printf("Non 5-bit stuffing not supported\n");
                if(ret == MAGIC)
                        printf("Invalid header of file\n");

                close(inputFile);
                return 0;
        }

        x       = ah.x_size;
        y       = ah.y_size;
        fps     = ah.fps;
        frames  = le16toh(ah.frames);
        /*obtain the properties of the file*/

        strm.zalloc     = Z_NULL;
        strm.zfree      = Z_NULL;
        strm.opaque     = Z_NULL;
        strm.avail_in   = 0;
        strm.next_in    = Z_NULL;
        /*prepare the zlib stream*/

        int infRet;
        if((infRet = inflateInit2(&strm, 31)) != Z_OK ){
        /*initialize the decompression stream*/

                char* f = "Failed to initialize decompression: %s\n";
                if(infRet == Z_MEM_ERROR)
                        printf(f, "Not enough memory");
                else if(infRet == Z_VERSION_ERROR)
                        printf(f, "Version differences");
                close(inputFile);
                return 1;
        }

        if(frames == 1){
        /*if the frame count is only 1, then it's not a video, its a single
        *picture, so just draw it and exit
        */
                decompressAndDisplay(inputFile, &strm, x, y);
                close(inputFile);
                inflateEnd(&strm);
                return 0;
        }

        struct timeval start, stop;
        uint8_t* frame;


        /*calculate the time between each frame display by rendering
        *the first frame
        */
        int skipFrame = 0;
        uint16_t frameBuffLen = x * y;
        while(frameBuffLen % BYTE_ELEMENT_SIZE != 0)
                frameBuffLen++;
        frameBuffLen/=BYTE_ELEMENT_SIZE;
        /*get the number of bit-compressed elements in the buffer by garunteeing
        *that there is enough elements  to fully hold all elements.*/

        for(uint i = 0; i < frames; i++){
        /*decompress all the frames in order*/

                gettimeofday(&start, NULL);
                frame = decompress(inputFile, &strm, frameBuffLen);
                if(skipFrame){
                        skipFrame--;
                        free(frame);
                        continue;
                }

                clear();

                display(frame, x, frameBuffLen);
                free(frame);

                gettimeofday(&stop, NULL);
		frameDuration = DURATION(start, stop);
		/*Calculate time it took to render that frame*/
		signed long long difference = (1000000 / (int8_t)fps - (signed)frameDuration);
		if(difference < 0){
			/*if the time to render the frame is longer than the fps rate allows, skip waiting*/
			skipFrame = ceil( (-1.0) * (double)difference / (1000000.0 / (double)fps) );
                        long wait = skipFrame * (1000000 / fps) - frameDuration;
                        usleep(abs(wait));
                        continue;
                }

                usleep(1000000 / fps - frameDuration);
                /*sleep for the duration that the frame is visable*/

        }



        printf("\n\n\n");
        /*jump to new line*/


        close(inputFile);
        inflateEnd(&strm);
        /*close the files and end the stream*/
        return 0;
}

/***************************************************************************
* enum Header_Error processFileErrors(ASCIIheader a)
* Author: SkibbleBip
* Date: 08/18/2021
* Description: function that handles file processing errors on the input video
*               file
*
* Parameters:
*        a      I/P     ASCIIheader     struct of the file header of the input
*                                               video file to be read
*        processFileErrors      O/P     enum Header_Error       return error
*                                                                       code
**************************************************************************/
enum Header_Error processFileErrors(ASCIIheader a)
{

        if(strncmp((char*)a.header_field, "ASCII", strlen("ASCII")))
                return MAGIC;

        if(a.compression != 1)
                return COMPRESSION;

        return OK;

}

/***************************************************************************
* uint8_t* decompress(int input, z_stream* s, uint8_t x, uint8_t y)
* Author: SkibbleBip
* Date: 04/10/2023
* Description: Function that decompresses a single ASCII rendered frame and
* 		returns it. Frame buffer will be of size array_list
*
* Parameters:
*        input  I/P     int             File descriptor to the input video file
*        s      I/O     z_stream*       pointer to zlib stream compressor
*        array_size     I/P     uint16_t         x and y size of frame
*        decompress	O/P	uint8_t*		Pointer to ASCII frame array
**************************************************************************/
uint8_t* decompress(int input, z_stream* s, uint16_t array_size)
{
        static uint8_t raw[READ_BUFFER_SIZE];
        /*raw byte read from the input file.
        * this HAS to be static, because sometimes the input buffer doesn't get
        * flushed and needs to be reused. As a result, this raw buffer needs
        * to be static so any previously read data is reusable, if needed.*/

        uint16_t pos = 0;
        /*position along the buffer where it's been read to*/

        uint8_t* bit_compressed    = malloc(array_size);
        uint8_t* bit_compressedTmp = malloc(array_size);
        /*buffers to contain the temporary bit-stuffed chunks, and the total
        *buffer to contain the entire frame
        */


        if(!bit_compressed || !bit_compressedTmp){
        /*check if the dynamic buffers couldnt be allocated*/
                perror("Failed to allocate timing frame");
                close(input);
                inflateEnd(s);
                free(bit_compressed);
                free(bit_compressedTmp);
                exit(1);
        }

        int ret;
        /*return code of the inflation engine*/

        uint16_t have;


        do{

                int32_t readBytes = s->avail_in;
                uint16_t len;

                if(s->avail_in == 0){
                        /*Only read if available in is 0. If it isn't, we have to re-scan previous data*/
                        if( (readBytes = read(input, &raw, READ_BUFFER_SIZE) ) < 0){
                                /*read the byte from the input file*/
                                fprintf(stderr, "Failed to read file\n");
                                close(input);
                                inflateEnd(s);
                                exit(1);
                        }
                        if(readBytes == 0){
                                /*Something has gone wrong reading the file,
                                we've reached EOF before we are done processing
                                */
                                fprintf(stderr, "Early EOF reached\n");
                                break;
                        }
                        s->next_in  = raw;

                }
                s->avail_in = (unsigned)readBytes;
                /*pointer to the byte being read*/

                do{

                        len = array_size - pos;
                        /*calculate the length of the buffer to be filled*/
                        if(len == 0)
                                break;
                        /*if the length to read the remaining frame is 0, then
                        *break out, because there is no more remaining data to
                        *fill
                        */

                        s->avail_out = len;
                        s->next_out  = bit_compressedTmp;
                        /*set the output buffer to point to the tmp buffer and
                        *it's length of the remaining unused and unfilled room
                        */


                        ret = inflate(s, Z_NO_FLUSH);
                        /*inflate the buffer*/

                        if(ret < Z_OK){
                                /*check if failed to decompress*/
                                char* q = "Decompression error-%s: %s";
                                char* p;

                                switch(ret){
                                case Z_ERRNO: p="File operation error";
                                break;
                                case Z_STREAM_ERROR: p="Stream error";
                                break;
                                case Z_DATA_ERROR: p="Data error";
                                break;
                                case Z_MEM_ERROR: p="Memory error";
                                break;
                                case Z_BUF_ERROR: p="Buffer error";
                                break;
                                case Z_VERSION_ERROR: p="Zlib Version error";
                                break;
                                default: p = "Unspecified error";
                                break;

                                }


                                fprintf(stderr, q, p, s->msg);
                                /*formulate and display the error message*/

                                close(input);
                                inflateEnd(s);
                                free(bit_compressed);
                                free(bit_compressedTmp);
                                exit(1);
                        }

                        have = len - s->avail_out;
                        /*get length of newly filled bytes*/

                        memcpy(bit_compressed+pos, bit_compressedTmp, have);
                        /*copy the bytes to the full buffer from the tmp buff
                        *and copy the bytes that were read to the end of the
                        *current buffer
                        */
                        pos+=have;
                        /*add the number of read bytes to the position value*/


                }while(s->avail_out == 0);
                /*keep reading clumps*/

        }while(pos < array_size);
        /*continue until the position variable is smaller than the size
        *of the total buffer
        */

        if(pos != array_size){
                /*sanity check if the position value somehow expanded past the size of
                *the buffer
                */
                fprintf(stderr, "Buffer not fully filled\n");
                close(input);
                inflateEnd(s);
                free(bit_compressedTmp);
                free(bit_compressed);
                exit(1);
        }
        free(bit_compressedTmp);
        return bit_compressed;
        /*free the compressed buffers*/

}


/***************************************************************************
* void display(uint8_t* frame, uint16_t array_size)
* Author: SkibbleBip
* Date: 04/10/2023
* Description: Function that prints a single ASCII rendered frame
*
* Parameters:
*        frame	I/P	uint8_t*		Pointer to ASCII frame array
*        x		I/P uint8_t			Horizontal width of a frame
*        array_size I/P uint16_t	size of frame buffer
**************************************************************************/
void display(uint8_t* frame, uint8_t x, uint16_t array_size)
{
        uint16_t pos = 5;

        char tmp[6];
        uint8_t location = frame[0];
        memcpy(tmp, lookup_table[location], 5);
        tmp[5] = '\000';
        fputs(tmp, stdout);
        /*print the very first 5 ASCII pixels*/

        for(uint16_t i = 1; i < array_size; i++){
        /*loop through all the elements of the bit-stuffed buffer and get it's
        *actual 5-char ASCII pixel from the lookup table
        */
                if(pos%x==0)
                        fputs("\n", stdout);
                /*if at the end of a horizontal line, jump to next line*/

                location = frame[i];
                memcpy(tmp, lookup_table[location], 5);
                tmp[5] = '\000';
                fputs(tmp, stdout);
                pos+=5;
                /*get the 5 ASCII pixels from the byte value from the lookup
                *table
                */
        }

}

/***************************************************************************
* unsigned long calcFPSDuration(int input, z_stream* s, uint8_t x, uint8_t y)
* Author: SkibbleBip
* Date: 08/25/2021
* Description: Function that calculated the duration between frames being
*               rendered by calling decompressAndDisplay on one frame and
*               measuring the duration it took to complete
*
* Parameters:
*        input  I/P     int             File descriptor to the input video file
*        s      I/O     z_stream*       pointer to zlib stream compressor
*        x      I/P     uint8_t         x width of frame
*        y      I/P     uint8_t         y height of frame
**************************************************************************/
//unsigned long calcFPSDuration(int input, z_stream* s, uint8_t x, uint8_t y)
//{
//        struct timeval start, stop;
//
//        gettimeofday(&start, NULL);
//        /*get start time*/
//
//        decompressAndDisplay(input, s, x, y);
//        /*decompress a single frame and display it*/
//
//        gettimeofday(&stop, NULL);
//        /*get the stop time*/
//
//        unsigned long duration = (stop.tv_sec - start.tv_sec)
//                        * 1000000 + stop.tv_usec - start.tv_usec;
//        /*get the duration in microseconds between the start and stop times*/
//
//        return duration;
//        /*return the duration in microseconds*/
//
//}


/***************************************************************************
* void decompressAndDisplay(int input, z_stream* s, uint8_t x, uint8_t y)
* Author: SkibbleBip
* Date: 08/25/2021
* Description: Function that decompresses a single ASCII rendered frame and
*               displays it
*
* Parameters:
*        input  I/P     int             File descriptor to the input video file
*        s      I/O     z_stream*       pointer to zlib stream compressor
*        x      I/P     uint8_t         x width of frame
*        y      I/P     uint8_t         y height of frame
**************************************************************************/
void decompressAndDisplay(int input, z_stream* s, uint8_t x, uint8_t y)
{
	uint16_t frameBuffLen = x * y;
        while(frameBuffLen % BYTE_ELEMENT_SIZE != 0)
                frameBuffLen++;
        frameBuffLen/=BYTE_ELEMENT_SIZE;
        /*get the number of bit-compressed elements in the buffer by garunteeing
        *that there is enough elements  to fully hold all elements.*/
	uint8_t* frame = decompress(input, s, frameBuffLen);
	display(frame, x, frameBuffLen);
	free(frame);

}

