
CC := gcc -g -Wall -pedantic -lmagic

yacuws: yacuws.c
	$(CC) yacuws.c -o yacuws

