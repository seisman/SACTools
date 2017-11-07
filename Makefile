CFLAGS = -Wall

BIN = ${HOME}/bin

all: sac2col sacch saclh sacmax clean

sac2col: sac2col.o sacio.o
	$(CC) -o $(BIN)/$@ $^

sacch: sacch.o sacio.o datetime.o
	$(CC) -o $(BIN)/$@ $^ -lm

saclh: saclh.o sacio.o
	$(CC) -o $(BIN)/$@ $^

sacmax: sacmax.o sacio.o
	$(CC) -o $(BIN)/$@ $^

clean:
	rm *.o
