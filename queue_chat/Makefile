BIN = binar
OBJ = object

.PHONY: all clean

all: sender server_new

clean:
	rm -f $(OBJ)/*.o $(BIN)/*

sender.o: sender.c
	gcc -c -o $(OBJ)/sender.o sender.c -lrt

server_new.o: server_new.c
	gcc -c -o $(OBJ)/server_new.o server_new.c -lrt

sender: sender.o
	gcc -o $(BIN)/sender $(OBJ)/sender.o -lrt

server_new: server_new.o
	gcc -o $(BIN)/server_new $(OBJ)/server_new.o -lrt
