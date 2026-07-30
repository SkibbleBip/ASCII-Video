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
* main                  -main driver function
***************************************************************************/


#include <stdio.h>
#include <stdlib.h>


#include "../ASCII.h"



/*function prototypes*/
void printer(const char* str)
{
    fputs(str, stdout);
}

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

        ASCIIfileHandle handle = {'\0'};
        int ret = openASCIIvideo(args[1], &handle, printer);
        if(ret)
            return ret;
        for(uint16_t idx = handle.header.frames; idx > 0; idx = renderFrame(&handle, 1.0f))
            ;


        /*ret = playFile(&handle);
        if(ret)
            return ret;
        closeASCIIvideo(&handle);*/
        return 0;
}




