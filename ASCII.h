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
* File:  ASCII.h
* Author:  SkibbleBip
* Procedures: N/A
***************************************************************************/
#ifndef BMP_H
#define BMP_H

#include <stdint.h>


#define         BMP_HEADER_SIZE         14
#define         DIB_HEADER_SIZE         40
/*size of BMP header properties*/

/**properties of byte compression**/
#define         BYTE_ELEMENT_SIZE       5
/*number of elements that can be stuffed into a byte*/

#define         COMPRESS_NONE           0
#define         COMPRESS_5_BIT          1
#define         COMPRESS_BITSHIFT       2
/*Compression types*/
/***********************************/



/*IDs per each colour (black is a space, grey is *, white is #)*/
#define         BLACK                   0
#define         GREY                    1
#define         WHITE                   2

enum Header_Error {OK, COMPRESSION, MAGIC};
/*definition for processing errors for the player*/


typedef struct{
        uint16_t padding;
        uint16_t header_field;
        uint32_t size;
        uint32_t reserved;
        uint32_t offset;

} BMP_header;
/*The header of the BMP file*/

typedef struct{
        uint32_t header_size;
        int32_t  width;
        int32_t  height;
        uint16_t colour_planes;
        uint16_t bits_per_pixels;
        uint32_t compression;
        uint32_t image_size;
        uint32_t hor_resol;
        uint32_t vert_resol;
        uint32_t num_of_colours;
        uint32_t num_important_colours;


} DIB_header;
//BITMAPINFOHEADER

typedef struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
} pixel;
/*RGB values of a single pixel*/


typedef struct {

        uint8_t header_field[5];/* "ASCII", magic number */
        uint8_t compression;/*Compression algorythm, 0:none, 1:5-bit, 2:bitshifted*/
        uint16_t frames;        /*how many frames are contained in the file*/
        uint8_t fps;            /*frames per second*/
        uint8_t x_size;         /*Horizontal length of the video*/
        uint8_t y_size;         /*Vertical height of the video*/
        uint8_t reserved;       /*reserved for future use*/

/*Little Endian*/
} ASCIIheader;


const uint8_t lookup_table_size = 243;
/*size of the table elements*/
const char lookup_table[243][5] = {
/*lookup table to be used to decode the bit-shifted compressed data*/
        {' ', ' ', ' ', ' ', ' '}, {'*', ' ', ' ', ' ', ' '}, {'#', ' ', ' ', ' ', ' '}, {' ', '*', ' ', ' ', ' '},
        {'*', '*', ' ', ' ', ' '}, {'#', '*', ' ', ' ', ' '}, {' ', '#', ' ', ' ', ' '}, {'*', '#', ' ', ' ', ' '},
        {'#', '#', ' ', ' ', ' '}, {' ', ' ', '*', ' ', ' '}, {'*', ' ', '*', ' ', ' '}, {'#', ' ', '*', ' ', ' '},
        {' ', '*', '*', ' ', ' '}, {'*', '*', '*', ' ', ' '}, {'#', '*', '*', ' ', ' '}, {' ', '#', '*', ' ', ' '},
        {'*', '#', '*', ' ', ' '}, {'#', '#', '*', ' ', ' '}, {' ', ' ', '#', ' ', ' '}, {'*', ' ', '#', ' ', ' '},
        {'#', ' ', '#', ' ', ' '}, {' ', '*', '#', ' ', ' '}, {'*', '*', '#', ' ', ' '}, {'#', '*', '#', ' ', ' '},
        {' ', '#', '#', ' ', ' '}, {'*', '#', '#', ' ', ' '}, {'#', '#', '#', ' ', ' '}, {' ', ' ', ' ', '*', ' '},
        {'*', ' ', ' ', '*', ' '}, {'#', ' ', ' ', '*', ' '}, {' ', '*', ' ', '*', ' '}, {'*', '*', ' ', '*', ' '},
        {'#', '*', ' ', '*', ' '}, {' ', '#', ' ', '*', ' '}, {'*', '#', ' ', '*', ' '}, {'#', '#', ' ', '*', ' '},
        {' ', ' ', '*', '*', ' '}, {'*', ' ', '*', '*', ' '}, {'#', ' ', '*', '*', ' '}, {' ', '*', '*', '*', ' '},
        {'*', '*', '*', '*', ' '}, {'#', '*', '*', '*', ' '}, {' ', '#', '*', '*', ' '}, {'*', '#', '*', '*', ' '},
        {'#', '#', '*', '*', ' '}, {' ', ' ', '#', '*', ' '}, {'*', ' ', '#', '*', ' '}, {'#', ' ', '#', '*', ' '},
        {' ', '*', '#', '*', ' '}, {'*', '*', '#', '*', ' '}, {'#', '*', '#', '*', ' '}, {' ', '#', '#', '*', ' '},
        {'*', '#', '#', '*', ' '}, {'#', '#', '#', '*', ' '}, {' ', ' ', ' ', '#', ' '}, {'*', ' ', ' ', '#', ' '},
        {'#', ' ', ' ', '#', ' '}, {' ', '*', ' ', '#', ' '}, {'*', '*', ' ', '#', ' '}, {'#', '*', ' ', '#', ' '},
        {' ', '#', ' ', '#', ' '}, {'*', '#', ' ', '#', ' '}, {'#', '#', ' ', '#', ' '}, {' ', ' ', '*', '#', ' '},
        {'*', ' ', '*', '#', ' '}, {'#', ' ', '*', '#', ' '}, {' ', '*', '*', '#', ' '}, {'*', '*', '*', '#', ' '},
        {'#', '*', '*', '#', ' '}, {' ', '#', '*', '#', ' '}, {'*', '#', '*', '#', ' '}, {'#', '#', '*', '#', ' '},
        {' ', ' ', '#', '#', ' '}, {'*', ' ', '#', '#', ' '}, {'#', ' ', '#', '#', ' '}, {' ', '*', '#', '#', ' '},
        {'*', '*', '#', '#', ' '}, {'#', '*', '#', '#', ' '}, {' ', '#', '#', '#', ' '}, {'*', '#', '#', '#', ' '},
        {'#', '#', '#', '#', ' '}, {' ', ' ', ' ', ' ', '*'}, {'*', ' ', ' ', ' ', '*'}, {'#', ' ', ' ', ' ', '*'},
        {' ', '*', ' ', ' ', '*'}, {'*', '*', ' ', ' ', '*'}, {'#', '*', ' ', ' ', '*'}, {' ', '#', ' ', ' ', '*'},
        {'*', '#', ' ', ' ', '*'}, {'#', '#', ' ', ' ', '*'}, {' ', ' ', '*', ' ', '*'}, {'*', ' ', '*', ' ', '*'},
        {'#', ' ', '*', ' ', '*'}, {' ', '*', '*', ' ', '*'}, {'*', '*', '*', ' ', '*'}, {'#', '*', '*', ' ', '*'},
        {' ', '#', '*', ' ', '*'}, {'*', '#', '*', ' ', '*'}, {'#', '#', '*', ' ', '*'}, {' ', ' ', '#', ' ', '*'},
        {'*', ' ', '#', ' ', '*'}, {'#', ' ', '#', ' ', '*'}, {' ', '*', '#', ' ', '*'}, {'*', '*', '#', ' ', '*'},
        {'#', '*', '#', ' ', '*'}, {' ', '#', '#', ' ', '*'}, {'*', '#', '#', ' ', '*'}, {'#', '#', '#', ' ', '*'},
        {' ', ' ', ' ', '*', '*'}, {'*', ' ', ' ', '*', '*'}, {'#', ' ', ' ', '*', '*'}, {' ', '*', ' ', '*', '*'},
        {'*', '*', ' ', '*', '*'}, {'#', '*', ' ', '*', '*'}, {' ', '#', ' ', '*', '*'}, {'*', '#', ' ', '*', '*'},
        {'#', '#', ' ', '*', '*'}, {' ', ' ', '*', '*', '*'}, {'*', ' ', '*', '*', '*'}, {'#', ' ', '*', '*', '*'},
        {' ', '*', '*', '*', '*'}, {'*', '*', '*', '*', '*'}, {'#', '*', '*', '*', '*'}, {' ', '#', '*', '*', '*'},
        {'*', '#', '*', '*', '*'}, {'#', '#', '*', '*', '*'}, {' ', ' ', '#', '*', '*'}, {'*', ' ', '#', '*', '*'},
        {'#', ' ', '#', '*', '*'}, {' ', '*', '#', '*', '*'}, {'*', '*', '#', '*', '*'}, {'#', '*', '#', '*', '*'},
        {' ', '#', '#', '*', '*'}, {'*', '#', '#', '*', '*'}, {'#', '#', '#', '*', '*'}, {' ', ' ', ' ', '#', '*'},
        {'*', ' ', ' ', '#', '*'}, {'#', ' ', ' ', '#', '*'}, {' ', '*', ' ', '#', '*'}, {'*', '*', ' ', '#', '*'},
        {'#', '*', ' ', '#', '*'}, {' ', '#', ' ', '#', '*'}, {'*', '#', ' ', '#', '*'}, {'#', '#', ' ', '#', '*'},
        {' ', ' ', '*', '#', '*'}, {'*', ' ', '*', '#', '*'}, {'#', ' ', '*', '#', '*'}, {' ', '*', '*', '#', '*'},
        {'*', '*', '*', '#', '*'}, {'#', '*', '*', '#', '*'}, {' ', '#', '*', '#', '*'}, {'*', '#', '*', '#', '*'},
        {'#', '#', '*', '#', '*'}, {' ', ' ', '#', '#', '*'}, {'*', ' ', '#', '#', '*'}, {'#', ' ', '#', '#', '*'},
        {' ', '*', '#', '#', '*'}, {'*', '*', '#', '#', '*'}, {'#', '*', '#', '#', '*'}, {' ', '#', '#', '#', '*'},
        {'*', '#', '#', '#', '*'}, {'#', '#', '#', '#', '*'}, {' ', ' ', ' ', ' ', '#'}, {'*', ' ', ' ', ' ', '#'},
        {'#', ' ', ' ', ' ', '#'}, {' ', '*', ' ', ' ', '#'}, {'*', '*', ' ', ' ', '#'}, {'#', '*', ' ', ' ', '#'},
        {' ', '#', ' ', ' ', '#'}, {'*', '#', ' ', ' ', '#'}, {'#', '#', ' ', ' ', '#'}, {' ', ' ', '*', ' ', '#'},
        {'*', ' ', '*', ' ', '#'}, {'#', ' ', '*', ' ', '#'}, {' ', '*', '*', ' ', '#'}, {'*', '*', '*', ' ', '#'},
        {'#', '*', '*', ' ', '#'}, {' ', '#', '*', ' ', '#'}, {'*', '#', '*', ' ', '#'}, {'#', '#', '*', ' ', '#'},
        {' ', ' ', '#', ' ', '#'}, {'*', ' ', '#', ' ', '#'}, {'#', ' ', '#', ' ', '#'}, {' ', '*', '#', ' ', '#'},
        {'*', '*', '#', ' ', '#'}, {'#', '*', '#', ' ', '#'}, {' ', '#', '#', ' ', '#'}, {'*', '#', '#', ' ', '#'},
        {'#', '#', '#', ' ', '#'}, {' ', ' ', ' ', '*', '#'}, {'*', ' ', ' ', '*', '#'}, {'#', ' ', ' ', '*', '#'},
        {' ', '*', ' ', '*', '#'}, {'*', '*', ' ', '*', '#'}, {'#', '*', ' ', '*', '#'}, {' ', '#', ' ', '*', '#'},
        {'*', '#', ' ', '*', '#'}, {'#', '#', ' ', '*', '#'}, {' ', ' ', '*', '*', '#'}, {'*', ' ', '*', '*', '#'},
        {'#', ' ', '*', '*', '#'}, {' ', '*', '*', '*', '#'}, {'*', '*', '*', '*', '#'}, {'#', '*', '*', '*', '#'},
        {' ', '#', '*', '*', '#'}, {'*', '#', '*', '*', '#'}, {'#', '#', '*', '*', '#'}, {' ', ' ', '#', '*', '#'},
        {'*', ' ', '#', '*', '#'}, {'#', ' ', '#', '*', '#'}, {' ', '*', '#', '*', '#'}, {'*', '*', '#', '*', '#'},
        {'#', '*', '#', '*', '#'}, {' ', '#', '#', '*', '#'}, {'*', '#', '#', '*', '#'}, {'#', '#', '#', '*', '#'},
        {' ', ' ', ' ', '#', '#'}, {'*', ' ', ' ', '#', '#'}, {'#', ' ', ' ', '#', '#'}, {' ', '*', ' ', '#', '#'},
        {'*', '*', ' ', '#', '#'}, {'#', '*', ' ', '#', '#'}, {' ', '#', ' ', '#', '#'}, {'*', '#', ' ', '#', '#'},
        {'#', '#', ' ', '#', '#'}, {' ', ' ', '*', '#', '#'}, {'*', ' ', '*', '#', '#'}, {'#', ' ', '*', '#', '#'},
        {' ', '*', '*', '#', '#'}, {'*', '*', '*', '#', '#'}, {'#', '*', '*', '#', '#'}, {' ', '#', '*', '#', '#'},
        {'*', '#', '*', '#', '#'}, {'#', '#', '*', '#', '#'}, {' ', ' ', '#', '#', '#'}, {'*', ' ', '#', '#', '#'},
        {'#', ' ', '#', '#', '#'}, {' ', '*', '#', '#', '#'}, {'*', '*', '#', '#', '#'}, {'#', '*', '#', '#', '#'},
        {' ', '#', '#', '#', '#'}, {'*', '#', '#', '#', '#'}, {'#', '#', '#', '#', '#'}
};



#endif
