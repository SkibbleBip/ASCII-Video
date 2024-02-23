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
* processFrame  -Function that takes in a file int of a BMP frame and processes
*       it to obtain a sampled frame in ASCII-format. It will then be further
*       compressed using bitwise stuffing and then piped through zlib and
*       written to the output file
* isLit         -function that determines what value the currently processed
*       pixel is represented by
* decode        -Decodes the input ASCII char to their ID eqivalent
* bitCompress   -bit padding compression for a 5-element char array, bit pads
*       the array and returns a single byte
* main          -The main driver function
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include <limits.h>
#include <sys/stat.h>
#include "../ASCII.h"


#define         WIDTH                   80
#define         HEIGHT                  30
/*width and height of the output video file*/

#define         VERSION         "2"



void processFrame(int tmp, z_stream* ptr, FILE* output);
uint8_t isLit(pixel p);
uint8_t decode(char in);
uint8_t bitCompress(char in[]);
/*Prototypes*/


/***************************************************************************
* int main(int argc, char* args[])
* Author: SkibbleBip
* Date: 08/15/2021
* Description: main function
*
* V1: Initial
* V2: Added input file support of individual bmp frames
*
* Parameters:
*        argc   I/P     int     count of command line params
*        args   I/P     char*[] array of command line parameters
*        main   O/P     int     return value of the program
**************************************************************************/
int main(int argc, char* args[])
{
        z_stream strm;

        char* dir                       = NULL;
        char* outputDir                 = NULL;
        /*location of the input dir and output file*/
        int frame                       = 0;
        /*file ID for the current frame*/
        FILE* vidFile                   = NULL;
        /*file pointer for the output file*/
        ASCIIheader asciiHeader         = {"ASCII", 1, 0, 0, WIDTH, HEIGHT, 0};
        uint8_t fps                     = 30;

        int ch;
        int iFlag = 0, oFlag = 0, eFlag = 0;
        while( (ch = getopt(argc, args, "hi:o:f:")) !=-1 && eFlag == 0){
        /*parse the command parameters*/
                switch(ch){
                /*depending on the inputted option, process the argument and
                *set the coresponding flag to present
                */

                        case 'i':{
                        /*declare parameter for -i */
                                iFlag = 1;
                                if(optarg[0] == '-'){
                                        eFlag = 1;
                                        break;
                                }

                                dir = optarg;
                                break;
                        }
                        case 'o':{
                        /*declare parameter for -o */
                                oFlag = 1;
                                outputDir = optarg;
                                break;
                        }
                        case 'h':{
                                iFlag = 1;
                                /*just pretend we typed -i*/
                                dir = (char*)1;
                                /*hack for making the dir not null*/
                                eFlag = 1;
                                break;

                        }
                        case 'f':{
                                fps = (uint8_t)atoi(optarg);
                                if(optarg[0] == '-'){
                                        eFlag = 1;
                                        break;
                                }
                                break;
                        }
                        default:{
                        /*display errors for any parameterless options or
                        *undefined options*/

                                if(optopt == 'i' || optopt == 'o' || optopt == 'f')
                                        fprintf(stderr, "Option -%c requires argument\n", optopt);
                                else
                                        fprintf(stderr, "Unknown option -%c\n", optopt);
                                eFlag = 1;
                                break;
                        }
                }
        }

        if(fps == 0){
                fprintf(stderr, "-f requires a nonzero parameter\n");
                eFlag = 1;
        }
        else if(!iFlag){
        /*if the option -i was never declared, display error*/
                fprintf(stderr, "Require option -i\n");
                eFlag = 1;
        }
        else if(dir == NULL){
        /*if -i was declared but no parameter was given*/
                fprintf(stderr, "-i requires a parameter\n");
                eFlag = 1;
        }
        if(oFlag && outputDir==NULL){
                fprintf(stderr, "-o requires a parameter\n");
                eFlag = 1;
        }

        if(eFlag){
        /*if there was an error, display proper usage and exit*/
                printf( "ASCII Video Generator Version " VERSION ", Copyright (C)"
                " 2021 SkibbleBip\nASCII VIdeo Generator comes with "
                "ABSOLUTELY NO WARRANTY; for details\ntype '-h'."
                "This is free software, and you are welcome\n"
                "to redistribute it under certain conditions.\n\n"
                        "\tUsage: %s -i [dir] -o [file] (-f [fps])\n\n", args[0]);
                return 0;
        }
        if(!oFlag){
        /*if no output file was declared, then default to "output.ASCII"*/
                outputDir = "output.ASCII";
                printf("Defaulting output file %s", outputDir);
        }

        strm.zalloc     = Z_NULL;
        strm.zfree      = Z_NULL;
        strm.opaque     = Z_NULL;

        int q;
        if( (q = deflateInit2 (&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                             Z_DEFAULT_STRATEGY) ) != Z_OK){

                char* e;
                if(q == Z_STREAM_ERROR)
                        e = "Invalid parameter.";
                if(q == Z_MEM_ERROR)
                        e = "Insufficient memory available.";
                if(q == Z_VERSION_ERROR)
                        e = "Version is not compatable.";

                fprintf(stderr, "Failed to init compression: %s\n", e);
                return q;

        }

        struct stat inpt;
        stat(dir, &inpt);
        char dest[PATH_MAX];
        char * destPtr;
        uint8_t flag;

        /*Check if the input location is a file or directory
        * if it is a directory, then get the path of the first frame
        */
        if(S_ISDIR(inpt.st_mode)){
            /* create the path of the first frame*/
            snprintf(dest, PATH_MAX, "%s/%03d.bmp", dir, 1);
            dest[PATH_MAX-1] = '\0';
            destPtr = dest;
            flag = 1;
        }
        else{
        /*if the path is just a single file, then get it's path as the input destination*/
            destPtr = dir;
            flag = 0;
        }

        if( (frame = open(destPtr, O_RDONLY)) < 0){
        /*open first frame, if it doesnt exist, exit*/
                fprintf(stderr, "Error, invalid frame %s\n", dest);
                return 0;
        }


        if( (vidFile = fopen(outputDir, "w+b")) == NULL){
        /*open the output file, handle any errors*/
                perror("Failed to open output file");
                exit(1);
        }

        asciiHeader.fps = fps;
        if(fwrite(&asciiHeader, 1, sizeof(ASCIIheader), vidFile ) != sizeof(ASCIIheader) ){
        /*handle writing error while writing the header of the file*/
                perror("Failed to write to video file");
                exit(1);
        }

        processFrame(frame, &strm, vidFile);
        printf("Processed frame 1...\n");
        /*process the first frame*/
        if(!flag){
            processFrame(-1, &strm, vidFile);
            /*processFrame performs the flushing when last file is reached.
            * hack for flushing the output
            */
            close(frame);
        }


        uint16_t c = 2;
        while(flag == 1){

                sprintf(dest, "%s/%03d.bmp", dir, c);
                frame = open(destPtr, O_RDONLY);
                /*get name of frame and open it*/

                processFrame(frame, &strm, vidFile);
                /*process the frame*/
                if(frame >= 0){
                /*if the frame was valid, then close it and increase the frame
                *count
                */
                        printf("Processed frame %d...\n", c);
                        close(frame);
                        c++;
                }
                else
                        flag = 0;
                /*otherwise, if the frame was invalid, then we are done
                *processing frames, set the completion flag to 0
                */


        }

        c--;
        uint16_t frameC = c;
        c = htole16(c);
        /*convert the frame count to Little Endian*/
        fseek(vidFile, strlen("ASCII")+1, SEEK_SET);
        /*go to the location to write the frame count*/
        fwrite(&c, 1, 2, vidFile);
        /*write the number of frames */

        printf("Writing to %s complete.\n"
                "\tTotal frames: %u Resolution: %u x %u\n\n",
                outputDir,
                frameC,
                WIDTH,
                HEIGHT );
        /*print resulting information*/

        fclose(vidFile);

        return 0;
}


/***************************************************************************
* void processFrame(int tmp, z_stream* ptr, FILE* output)
* Author: SkibbleBip
* Date: 08/13/2021
* Description: Function that takes in a file int of a BMP frame and processes it
* to obtain a sampled frame in ASCII-format. It will then be further compressed
* using bitwise stuffing and then piped through zlib and written to the output
* file
*
* V1: Initial
* V2: Fixed bug that would result in avail_in and next_in recycling input
* V3: Fixed slanting of the output frame when the input bmp image has a width
*   that is nondivisible by the default output width
*
* Parameters:
*        tmp    I/P     int     frame handle to process
*        ptr    I/O     z_stream*       pointer to the zlib stream struct
*        output I/O     FILE*   file pointer to the output file
**************************************************************************/
void processFrame(int tmp, z_stream* ptr, FILE* output)
{
        uint8_t pad             = 0;
        /*padding bytes in the bitmap frame*/
        uint32_t rowLen         = 0;
        /*length of reach row*/
        char buff[WIDTH * HEIGHT];
        /*temporary buffer that holds the uncompressed video*/
        uint8_t* compressed_buff;
        /*pointer to the compressed buffer*/
        char total[WIDTH][HEIGHT];
        /*uncompressed, literal frame*/
        BMP_header bmpHeader;
        DIB_header dibHeader;
        /*bitmap headers*/


        if(tmp < 0){
        /*if last frame was processed, then finish flushing the file and exit*/
                ptr->avail_in = 0;
                /*we dont have any available input*/
                ptr->next_in = (uint8_t*)total;
                /*need a valid pointer, but we wont be using this*/
                int ret;


                do{
                        ptr->avail_out = WIDTH*HEIGHT;
                        ptr->next_out = (Bytef*)buff;
                        /*declare the buffer to store the remaining bytes, if any*/
                        ret = deflate(ptr, Z_FINISH);
                        /*finish deflating*/

                        if(ret < 0){
                                /*if there was an error, display it and exit*/
                                fprintf(stderr, "Compression error: %s\n", ptr->msg);
                                deflateEnd(ptr);
                                exit(1);
                        }

                        uint remaining = WIDTH*HEIGHT - ptr->avail_out;
                        /*get number of bytes to write (can be 0)*/

                        if(fwrite(buff, 1, remaining, output) != remaining){
                                /*write compressed bytes and handle any errors*/
                                perror("Failed to write to output file");
                                deflateEnd(ptr);
                                exit(1);

                        }

                }while(ret != Z_STREAM_END);
                /*continue flushing output until stream lets us know
                we are done
                */


                printf("Total size: %lu bytes\n", ptr->total_out);
                /*print number of output bytes*/
                return;
        }

        /*EEEEEEEEEEEEEEEEEEEEWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW*/
        if(read(tmp, &((uint8_t*)&bmpHeader)[sizeof(bmpHeader.padding)], BMP_HEADER_SIZE) < 0){
        /*read the BMP header, handle any reading errors*/
                perror("Failed to read file");
                deflateEnd(ptr);
                exit(-1);
        }

        if(read(tmp, &dibHeader, DIB_HEADER_SIZE) < 0){
        /*read the DIB header, handle errors*/
                perror("Failed to read file");
                deflateEnd(ptr);
                exit(-1);
        }

        if(le16toh(dibHeader.bits_per_pixels) != 24){
        /*get the bits_per_pixel in Host-format, check if it's 24 bits. if not,
        *we cannot use the formated frame. exit
        */
                fprintf(stderr, "Non-24 bit BMPs are not supported\n");
                deflateEnd(ptr);
                exit(1);
        }


        uint32_t w;
        uint16_t bpp;
        w       = le32toh(dibHeader.width);
        bpp     = le16toh(dibHeader.bits_per_pixels);
        /*convert the byte orientation to the machine's orientation*/
        pad     = ((w * (bpp / 8)) % 4);
        /*calculate the padding of each pixel row*/
        rowLen  = w * (bpp / 8) + pad;
        /*calculate the length of each pixel row, including padding*/
        const int32_t col = (int32_t)le32toh(dibHeader.height);
        /*get height of the arrays*/
        const int32_t sample_w = (int32_t)le32toh(dibHeader.width) / WIDTH;
        const int32_t sample_h = (int32_t)le32toh(dibHeader.height) / HEIGHT;

        pixel pix;
        for(int32_t i = 0; i < col; i++){
        /*for every column, read in the bytes */
                for(uint j = 0; j < rowLen; j+=sample_w*3){
                        if(read(tmp, &pix, 3) < 1){
                        /*read a sampled pixel and process it*/
                                perror("Failed to read pixel");
                                deflateEnd(ptr);
                                exit(-1);
                        }
                        char b;
                        if(isLit(pix) == 1)
                                b = '#';
                        else if(isLit(pix) == 2)
                                b = '*';
                        else
                                b = ' ';
                        /*obtain the ASCII equivalent of the pixel*/

                        uint8_t x, y;
                        x = j / (3 * sample_w);
                        y = i / sample_h;
                        total[x][y] = b;

                        int32_t burner = (rowLen - j >= sample_w*3-3 ? sample_w*3-3 : rowLen - j);
                        uint8_t* buf = malloc(burner);

                        /*grab a sample buffer and burn it*/
                        if(read(tmp, buf, burner) < 1){
                                perror("Failed to resample");
                                deflateEnd(ptr);
                                exit(-1);
                        }

                        free(buf);

                }
                //for rowLen

                uint8_t burn;
                for(uint8_t j = 0; j < pad; j++)
                /*burn any buffer bytes*/
                        if(read(tmp, &burn, 1) < 0){
                                perror("Failed to read buffer data");
                                deflateEnd(ptr);
                                exit(-1);
                        }

                uint8_t* buf = malloc((sample_h -1) * rowLen );
                /*burn the horizontal buffer*/
                if(read(tmp, buf, (sample_h -1) * rowLen ) < 0){
                        perror("Failed to read horizontal buffer");
                        deflateEnd(ptr);
                        exit(-1);
                }
                free(buf);

                i+=(sample_h-1);

        }//for col

        uint16_t c = 0;
        for(int i = HEIGHT-1; i >=0; i--){
                for(int j = 0; j< WIDTH; j++){
                        buff[c++] = total[j][i];
                }
        }
        /*write bytes into uncompressed buffer*/


        uint16_t array_size = WIDTH * HEIGHT;
        while(array_size % BYTE_ELEMENT_SIZE != 0)
                array_size++;
        array_size/=BYTE_ELEMENT_SIZE;
        /*get the number of bit-compressed elements in the buffer by garunteeing
        *that there is enough elements  to fully hold all elements.*/
        compressed_buff = malloc(array_size);
        if(compressed_buff == NULL){
                perror("Failed to allocate temporary compression buffer");
                deflateEnd(ptr);
                exit(1);
        }
        /*allocate buffer to hold the compressed buffer*/

        uint16_t j = 0;
        for(uint16_t i = 0; i < array_size; i++){
                char clump[] = {' ',' ',' ',' ',' '};
                uint16_t remaining = WIDTH*HEIGHT-j < 5 ? WIDTH*HEIGHT - j : 5;
                /*decide if the remaining uncompressed bytes are large enough
                *to obtain a 5 byte sample, or just grab the remaining bytes
                */
                for(uint8_t q = 0; q < remaining; q++)
                        clump[q] = decode(buff[j++]);
                uint8_t val = bitCompress(clump);
                compressed_buff[i] = val;
                /*compress 5 characters into one byte, and procede to do that
                *for the entire uncompressed buffer
                */


        }

        ///BIT SHIFTING ALGORYTHM
        /*for(uint16_t i = 0; i < WIDTH*HEIGHT / 4; i++){
                compressed_buff[i] = 0;
                compressed_buff[i] |= decode(buff[j++]);
                compressed_buff[i] |= (decode(buff[j++]) << 2);
                compressed_buff[i] |= (decode(buff[j++]) << 4);
                compressed_buff[i] |= (decode(buff[j++]) << 6);

        }*/



        int ret;
        int remainingIn = array_size;
        do{
                ptr->avail_out = WIDTH*HEIGHT;
                ptr->next_out = (Bytef*)buff;
                /*reuse the buffer; tell the compresser to use it when zlib'ing
                the compressed data
                */

                ptr->avail_in = remainingIn;
                ptr->next_in = (Bytef*)compressed_buff;
                /*declare the buffer that stores the inputted data to be
                *zlib'd
                */


                ret = deflate(ptr, Z_NO_FLUSH);



                /*compress the data*/
                if(ret < 0 && ret != Z_BUF_ERROR){
                /*if compression error, return error*/
                        fprintf(stderr, "Compression error: %s\n", ptr->msg);
                        deflateEnd(ptr);
                        exit(1);
                }

                uint have = WIDTH*HEIGHT - ptr->avail_out;
                /*get the length of the bytes to write (can be 0)*/
                remainingIn = (ptr->avail_in ? remainingIn - ptr->avail_in : 0);

                if(fwrite(buff, 1, have, output) != have){
                /*write the compressed buffer to the output file, if any errors
                *are occured then return error
                */
                        perror("Failed to write to output file");
                        deflateEnd(ptr);
                        exit(1);

                }

        }while(ptr->avail_out == 0);
        /*loop until no bytes are left to write*/

        if(ptr->avail_in != 0){
        /*verify there are no bytes left to process (sanity check)*/
                fprintf(stderr, "bad compression");
                deflateEnd(ptr);
                free(compressed_buff);
                exit(1);
        }

        free(compressed_buff);
        /*free the compression buffer*/


}


/***************************************************************************
* uint8_t isLit(pixel p)
* Author: SkibbleBip
* Date: 08/10/2021
* Description: function that determines what value the currently processed pixel
* is represented by
*
* V1: Initial
* V2: Optimized calculating using bitwse operations
*
* Parameters:
*        p      I/P     pixel   pixel to analyze
*        isLit  O/P     uint8_t value that the pixel represents
**************************************************************************/
uint8_t isLit(pixel p)
{
        if( (p.blue | p.green | p.red) == 0)
                return 0;
        if((p.blue & p.green & p.red) == 255)
                return 1;
        else
                return 2;
}

/***************************************************************************
* uint8_t decode(char in)
* Author: SkibbleBip
* Date: 08/05/2021
* Description: Decodes the input ASCII char to their ID eqivalent
*
* Parameters:
*        in     I/P     char    input char to decode
*        decode O/P     uint8_t ID value represented by the inputted char
**************************************************************************/
uint8_t decode(char in)
{
        if(in == '#')
                return WHITE;
        if(in == '*')
                return GREY;
        return BLACK;
}

/***************************************************************************
* uint8_t bitCompress(char in[])
* Author: SkibbleBip
* Date: 08/15/2021
* Description: bit padding compression for a 5-element char array, bit pads the
* array and returns a single byte
*
* Parameters:
*        in     I/P     char[]          input 5-element array
*        bitCompress    O/P     uint8_t byte returned that contains the
*                                               compressed data
**************************************************************************/
uint8_t bitCompress(char in[])
{
        return in[0] + 3*(in[1]+3*(in[2]+3*(in[3]+3*(in[4]))));

}




