CC = gcc
CPPFLAG= 
CFLAGS = -Wall -std=c11 -g -Iinclude -fPIC
LDFLAGS = -L.
BIN = ../
SRC = src/

all:
	$(CC) $(CFLAGS) $(SRC)LinkedListAPI.c $(SRC)GEDCOMparser.c $(SRC)GEDCOMutilities.c -shared -o $(BIN)myLibrary.so
