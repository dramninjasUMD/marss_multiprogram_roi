.PHONY: all 

CC=gcc
LDFLAGS=-Wl,--no-as-needed -lrt
all: multiprogram_daemon unlock_test 

multiprogram_daemon: multiprogram_daemon.o affinity.o
unlock_test: unlock_test.o affinity.o

affinity.o: affinity.c
	$(CC) $(CFLAGS) -static -c -o affinity.o affinity.c
	
clean:
	-rm *.o multiprogram_daemon unlock_test

