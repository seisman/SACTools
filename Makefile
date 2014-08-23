CC = gcc
CFLAGS = -Wall

BIN = ${HOME}/bin

all: sac2col sacch

sac2col: sac2col.o sacio.o
	$(CC) -o $(BIN)/$@ $^

sacch: sacch.o sacio.o
	$(CC) -o $(BIN)/$@ $^ -lm

clean:
	rm *.o
