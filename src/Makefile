cracker: main.c
	gcc -o cracker main.c -lpthread -std=c99
clean:
	rm cracker
tests: cracker 02_6c_5.bin
	./cracker -c -t 5 02_6c_5.bin
all: cracker tests
