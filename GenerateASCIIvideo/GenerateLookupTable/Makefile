OBJS	= lookup.o
SOURCE	= lookup.c
HEADER	= 
OUT	= LookupTableGenerator
CC	 = gcc
FLAGS	 = -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LFLAGS)

lookup.o: lookup.c
	$(CC) $(FLAGS) lookup.c 


clean:
	rm -f $(OBJS) $(OUT)
