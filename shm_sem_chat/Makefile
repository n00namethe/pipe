BIN = binar
OBJ = object

.PHONY: all clean

all: shm_client shm_server

clean:
	rm -f $(OBJ)/*.o $(BIN)/*

shm_client.o: shm_client.c
	gcc -c -o $(OBJ)/shm_client.o shm_client.c -lrt -pthread -g

shm_server.o: shm_server.c
	gcc -c -o $(OBJ)/shm_server.o shm_server.c -lrt -pthread -g

shm_client: shm_client.o
	gcc -o $(BIN)/shm_client $(OBJ)/shm_client.o -lrt -pthread -g

shm_server: shm_server.o
	gcc -o $(BIN)/shm_server $(OBJ)/shm_server.o -lrt -pthread -g
