FLAGS=-DDEBUG
SRC=kradzieje.cpp
TARGET=kradzieje
all: main

main: kradzieje.cpp main.h 
	mpic++ $(SRC) $(FLAGS4) -o $(TARGET)

run: main
	mpirun -np 8 --oversubscribe kradzieje 8
	#mpirun -H localhost:4,lab-sec-18:4 -np 8 kradzieje 8

clean:
	rm -f kradzieje a.out
