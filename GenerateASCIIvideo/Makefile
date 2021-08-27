#**************************************************************************
#Copyright (C) 2021 SkibbleBip
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either version 2
#of the License, or (at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#**************************************************************************/



OBJS	= main.o
SOURCE	= main.c
HEADER	= ../ASCII.h
OUT	= ASCIIvideoGenerator
CC	 = gcc
FLAGS	 = -O3 -c -Wall
LFLAGS	 = -lz
DIR	 = build

all: 	$(OBJS)
	$(CC) $(DIR)/$(OBJS) -o $(DIR)/$(OUT) $(LFLAGS)

main.o: main.c
	mkdir -p $(DIR)
	$(CC) $(FLAGS) main.c
	mv $(OBJS) $(DIR)/

clean:
	rm -r-f $(OBJS) $(OUT)
