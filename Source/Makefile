CC=gcc
CFLAGS=-Wall -g

all: argus argusd

argus: writetxt.o argus.o readln.o tratar.o

argusd: writetxt.o argusd.o readln.o

writetxt.o:writetxt.c writetxt.h

readln.o:readln.c readln.h

tratar.o:tratar.c tratar.h

argus.o:argus.c

argusd.o:argusd.c


clean:
	rm *.o  fifo fifo1 log argusd argus log.idx