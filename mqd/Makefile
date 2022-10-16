SEND = mqd_send
RECIVE = mqd_recive
BIN = binar
OBJ = object

.PHONY: all clean

all: $(SEND) $(RECIVE)

clean:
	rm -rf $(OBJ)/*.o

mqd_send.o: mqd_send.c
	gcc -c -o $(OBJ)/mqd_send.o mqd_send.c

mqd_recive.o: mqd_recive.c
	gcc -c -o $(OBJ)/mqd_recive.o mqd_recive.c

$(SEND): mqd_send.o
	gcc -o $(BIN)/mqd_send $(OBJ)/mqd_send.o 

$(RECIVE): mqd_recive.o
	gcc -o $(BIN)/mqd_recive $(OBJ)/mqd_recive.o