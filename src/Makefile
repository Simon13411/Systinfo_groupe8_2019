
T = -t 4
C = -c 

all: cracker

cracker:main.o
	gcc -o cracker main.o

main.o:	main.c
	gcc -o main.o -c main.c

exemple1:
	@./cracker $(T) $(C) ./LOL.txt ./pasexiste.txt

