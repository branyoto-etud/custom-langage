# Makefile Projet Compilation

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite
CC=gcc
CFLAGS=-Wall 
SRC=projet
EXEC=compil

$(EXEC): $(SRC).o $(SRC).lex.o abstract-tree.o symbol-table.o trad-nasm.o
	gcc $(CFLAGS) $^ -o $@

abstract-tree.o: src/abstract-tree.c
	$(CC) -c $< $(CFLAGS) -std=c99

symbol-table.o: src/symbol-table.c
	$(CC) -c $< $(CFLAGS) -std=c99

trad-nasm.o: src/trad-nasm.c
	$(CC) -c $< $(CFLAGS) -std=c99

%.o: %.c
	$(CC) -c $< $(CFLAGS)

$(SRC).lex.c: src/$(SRC).lex $(SRC).tab.h
	flex -o $@ $<

$(SRC).c: src/$(SRC).y
	bison -o $@ $< --defines=$(SRC).tab.h

clean:
	rm -f *.c *.h *.o

mrproper: clean
	rm -f ./bin/$(EXEC)

install: $(EXEC)
	mkdir ./bin
	mv $(EXEC) ./bin
	make clean

uninstall: mrproper
	rm -df ./bin

all: uninstall install
