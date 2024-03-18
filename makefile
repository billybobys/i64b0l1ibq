all: drone.exe

drone.exe: main.o func.o
	gcc -o main main.o func.o

main.o: main.c func.h
	gcc -c -o main.o main.c -lncurses

func.o: func.c
	gcc -c -o func.o func.c -lncurses