FLAGS=-DDEBUG -lmpi_cxx -lmpi -lstdc++
FLAGS2=-lmpi_cxx -lmpi -lstdc++
SRC=kradzieje.cpp
TARGET=kradzieje
all: main

main: kradzieje.cpp main.h 
	mpicc $(SRC) $(FLAGS2) -o $(TARGET)

run: main
	mpirun -np 4 --oversubscribe kradzieje

clean:
	rm -f kradzieje a.out
