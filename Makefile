ASYNC_FILE_IO=src/async_file_io.c
build:
	gcc -Wall -g -std=c99 -levent $(ASYNC_FILE_IO) main.c -o main

all: build


build-asm:
	gcc -c -g assem.s && ld assem.o -o assem && rm assem.o 