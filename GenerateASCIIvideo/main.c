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
//#include <fcntl.h>
//#include <endian.h>
//#include <stdint.h>
//#include <string.h>
//#include <zlib.h>
//#include <limits.h>
//#include <sys/stat.h>
#include "../ASCII.h"




#define         VERSION         "2"




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


        char* dir                       = NULL;
        char* outputDir                 = NULL;
        /*location of the input dir and output file*/


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

        return createVideoFile(dir, outputDir, fps);

}







