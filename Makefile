ALL: 6502.o 6502 test

6502.o: 6502.s
	ca65 6502.s

6502: 6502.s 6502.o
	ld65 6502.o -o 6502.bin -C custom.cfg

test:
	gcc -o tests ./tests.c
