CC = gcc
CFLAGS = -Wall

BIN = ${HOME}/bin

all: sac2col

sac2col: sac2col.o sacio.o
	$(CC) -o $(BIN)/$@ $^

clean:
	rm *.o
