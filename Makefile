# LastEmperor, a Chess960 movegen tool (Derived from Sapeli 1.67)
# Copyright (C) 2019-2020 Toni Helminen
# GPLv3

# Compilers: [gcc, g++, clang, clang++]
CC=gcc
OPT=-O3 -march=native
FLAGS=-Wall -pedantic -Wextra -Wshadow $(OPT) -DNDEBUG
FILES=LastEmperor.c
EXE=lastemperor

all:
	$(CC) $(FLAGS) $(FILES) -o $(EXE)

bench:
	./$(EXE) -bench

perft:
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 11

install:
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

clean:
	rm -f a.out lastemperor*

.PHONY: all strip clean install bench
