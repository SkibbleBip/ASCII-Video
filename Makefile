
#OBJS	 = main.o
#SOURCE	 = main.c
HEADER	 = ASCII.h
#OUT	 = ASCIIplayer
GEN	 = ASCIIvideoGenerator
PLAY	 = ASCIIplayer
CC	 = gcc
FLAGS	 = -O3 -c -Wall
LFLAGS	 = -lz
BDIR	 = build
GENDIR	 = GenerateASCIIvideo
PLAYDIR	 = ASCII\ Player
GEX	 = $(GENDIR)/build/$(GEN)
PEX	 = $(PLAYDIR)/build/$(PLAY)

all: 	#$(OBJS)
	#$(CC) $(DIR)/$(OBJS) -o $(DIR)/$(OUT) $(LFLAGS)
	(cd $(GENDIR); make)
	(cd $(PLAYDIR); make)
	mkdir -p $(BDIR)
	cp $(GEX) $(BDIR)/$(GEN)
	cp $(PEX) $(BDIR)/$(PLAY)



clean:
	rm -r-f $(GENDIR) $(BDIR)
