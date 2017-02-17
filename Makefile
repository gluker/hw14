all : data_store.c assembler.c
	gcc -g -Wall -ansi -pedantic -o assembler assembler.c data_store.c 
