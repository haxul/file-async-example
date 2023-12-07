run-gcc:
	gcc -Wall -g -std=c99 -levent main.c -o main

all: run-gcc