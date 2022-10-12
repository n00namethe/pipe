.PHONY: all clean

all: fifo

clean:
	rm -rf build/*.o

fifo.o: fifo.c
	gcc -c -o build/fifo.o fifo.c

fifo1.o: fifo1.c
	gcc -c -o build/fifo1.o fifo1.c

fifo: fifo.o fifo1.o
	gcc -o build/fifo build/fifo.o && gcc -o build/fifo1 build/fifo1.o