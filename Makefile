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


#OBJS	 = main.o
#SOURCE	 = main.c
HEADER	 = ASCII.h
GEN	 = ASCIIvideoGenerator
PLAY	 = ASCIIplayer
FLAGS	 = $(EXTRAFLAGS) -O3 -c -Wall
LFLAGS	 = $(EXTRALFLAGS) -lz
BDIR	 = build
GENDIR	 = GenerateASCIIvideo
PLAYDIR	 = ASCII\ Player
GEX	 = $(GENDIR)/build/$(GEN)
PEX	 = $(PLAYDIR)/build/$(PLAY)

all: 	#$(OBJS)
	(cd $(GENDIR); make)
	(cd $(PLAYDIR); make)
	mkdir -p $(BDIR)
	cp $(GEX) $(BDIR)/$(GEN)
	cp $(PEX) $(BDIR)/$(PLAY)



clean:
	rm -r -f $(GENDIR)/build $(PLAYDIR)/build $(BDIR)
