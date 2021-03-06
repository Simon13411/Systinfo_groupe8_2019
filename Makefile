cracker: src/cracker.c
	gcc -o cracker src/cracker.c -lpthread -std=c99 -Wall -Werror
clean:
	rm cracker tests/fichierOut.txt

tests: tests1 tests2 tests2 tests3 tests4

tests1: cracker tests/02_6c_5.bin tests/01_4c_1k.bin
	./cracker -t 100 tests/02_6c_5.bin tests/01_4c_1k.bin
tests2: cracker tests/02_6c_5.bin 
	./cracker -t 4 tests/02_6c_5.bin
tests3: cracker tests/01_4c_1k.bin
	./cracker -t 100  -c tests/01_4c_1k.bin
tests4: cracker tests/01_4c_1k.bin
	./cracker -t 100 -o tests/fichierOut.txt tests/01_4c_1k.bin
all: cracker tests

