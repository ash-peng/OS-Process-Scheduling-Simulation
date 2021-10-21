CFLAGS = -Wall -g -std=c99 -Werror

all: build

build:
	gcc $(CFLAGS) simulator.c PCB.c list.o -o cpu_simulator

run: build
	./cpu_simulator

valgrind: build
	valgrind --leak-check=full ./cpu_simulator

clean:
	rm -f cpu_simulator