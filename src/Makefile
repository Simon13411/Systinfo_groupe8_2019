cracker: main.c
	gcc -o cracker main.c -lpthread -std=c99
clean:
	rm cracker
tests: cracker 02_6c_5.bin 01_4c_1k.bin
	./cracker -t 100 02_6c_5.bin 01_4c_1k.bin
all: cracker tests
