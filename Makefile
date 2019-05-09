cracker: src/main.c clean
	gcc -o cracker src/main.c -lpthread -std=c99 -Wall -Werror
clean:
	rm cracker
tests: cracker tests/02_6c_5.bin tests/01_4c_1k.bin
	./cracker -t 100 tests/02_6c_5.bin tests/01_4c_1k.bin
all: cracker tests
