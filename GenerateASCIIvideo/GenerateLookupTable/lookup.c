/***************************************************************************
* File:  lookup.c
* Author:  SkibbleBip
* Procedures:
* main  -The main processing function, displays the output when done processing
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>

/***************************************************************************
* int main(void)
* Author: SkibbleBip
* Date: 08/13/2021
* Description: The main function, displays the results
*
* Parameters:
*        main   O/P     int     return value of program
**************************************************************************/
int main(void){

        int str[] = {0,0,0,0,0};
        /*starting string*/

        printf(" = {\n\t");
        /*print the starting bracket*/

        int i = 0;
        /*current location of the index of the string*/
        int c = 1;
        /*count of the current combination*/
        int d = 1;
        /*distance along the horizontal the display of the array is at*/
        do{

                if(c == d){
                /*if the current element is a new start of a line, dont print
                *the comma
                */
                        printf("{");
                        d+=4;
                        /*add 4 to the distance the line has traveled*/
                }
                else
                /*otherwise print the leading comma*/
                        printf(", {");


                for(int j = 0; j < 5; j++){
                /*for each element in the string, print the pixel equivalent
                *ASCII char of the element
                */
                        char q;
                        if(str[j] == 0)
                                q = ' ';
                        else if(str[j] == 1)
                                q = '*';
                        else
                                q = '#';

                        if(j == 4)
                                printf("'%c'", q);
                        else
                                printf("'%c', ", q);
                }

                printf("}");
                /*print closing bracket of the element*/

                if(c % 4 == 0 )
                        printf(",\n\t");
                /*if we have just processed the 4Nth element, print comma and
                *start a new line
                */


                str[i]++;
                /*increment the value at the current index*/

                int b = 0;
                /*flag for breaking when done*/
                while(str[i] == 3){
                /*if the current element is 3 (too big) set it to 0 and
                *increment the next element and go up an index. repeat this
                *until no element is 3
                */
                        str[i] = 0;
                        i++;
                        str[i]++;
                        if(i == 4 && str[4] == 3){
                        /*if we reached the last element and the last element is
                        *3, we finished generating every possible combination,
                        *we must break out and end the application
                        */
                                //str[4]--;
                                b = 1;
                                break;
                        }

                }
                 if(b)
                 /*if break flag was triggered, break out again*/
                        break;

                i = 0;
                c++;

        }while(1);

        printf("\n};\n\n\n");
        /*print closing bracket of the total array*/

        printf("size: %d\n\n", c);
        /*print size of all strings in the array*/
        return 0;
        /*exit*/
}



