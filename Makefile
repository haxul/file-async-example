run-gcc:
	gcc -Wall -g -std=c99 -levent main.c -o main

all:
	clang  main.c -o main $(CLION_EXE_DIR)