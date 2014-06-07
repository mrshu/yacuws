
CC := gcc -g -Wall -pedantic

yacuws: yacuws.c
	$(CC) yacuws.c -o yacuws

