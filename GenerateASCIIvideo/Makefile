

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
