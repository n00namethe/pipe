BIN = binar
OBJ = object

.PHONY: all clean

all: mqd_send mqd_recive

clean:
	rm -f $(OBJ)/*.o $(BIN)/*

mqd_send.o: mqd_send.c
	gcc -c -o $(OBJ)/mqd_send.o mqd_send.c

mqd_recive.o: mqd_recive.c
	gcc -c -o $(OBJ)/mqd_recive.o mqd_recive.c

mqd_send: mqd_send.o
	gcc -o $(BIN)/mqd_send $(OBJ)/mqd_send.o 

mqd_recive: mqd_recive.o
	gcc -o $(BIN)/mqd_recive $(OBJ)/mqd_recive.o