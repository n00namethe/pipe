BIN = binar
OBJ = object

.PHONY: all clean

all: sender server

clean:
	rm -f $(OBJ)/*.o $(BIN)/*

sender.o: sender.c
	gcc -c -o $(OBJ)/sender.o sender.c

server.o: server.c
	gcc -c -o $(OBJ)/server.o server.c

sender: sender.o
	gcc -o $(BIN)/sender $(OBJ)/sender.o 

server: server.o
	gcc -o $(BIN)/server $(OBJ)/server.o
