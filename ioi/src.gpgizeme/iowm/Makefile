


CC = gcc
CFLAGS = -g
LDFLAGS = -static


all: clean iog

iog: iowm.h iowm_workload.h iowm.c main.c
	$(CC) -c $(CFLAGS) iowm.c
	$(CC) -c $(CFLAGS) main.c
	$(CC) -o $@ $(LDFLAGS) main.o iowm.o

clean:
	rm -f *.o iog
