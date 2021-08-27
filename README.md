# ASCII-VIDEO

## What is this?
This is a file formatting and video player to display monochromatic non-audio videos in ASCII format on the terminal. 
## How does this work? 
The video "encoder" and file generator takes in the location of a directory that contains each individual frame of the video saved as an uncompressed 24-bit BMP file in the name formatting as `%03d.bmp` (001.bmp, 002.bmp, 003.bmp...). Each frame then has it's pixels sampled to match a set resolution of 80 chars by 30 chars (height on most terminals is double the width so this matches an 80x60 resolution) and then compressed first using a 5-bit stuffing algorithm for 5 ASCII chars into a single byte (`char1 + 3*(char2+3*(char3+3*(char4+3*(char5))))`) and then further compressing it through Zlib's deflate algorithm. Information on frames, FPS, resolution and compression methods are stored in the header of the output ASCII video file. Information about the output ASCII video file is defined in ASCII.h . The 3 char "pixels" available in the video output is "black" represented by a space, "white" represented by a '#', and any other colour in between represented by a '*'.

## Usages
If a user were to make a terminal-based game and wanted a little cutscene, they could use this implementation of this video player. Also, I have gotten "video" to work on embedded devices that do not have traditional display peripherals such as routers or arduinos through the usage of UART or other serial connections; this video project can be implemented entirely esoterically.

## License
Copyright (C) 2021 SkibbleBip

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
