# Copyright (C) 2019-2020 Toni Helminen <GPLv3>

# Definitions

CC=clang
CFLAGS=-O2 -march=native -Wall -pedantic -Wextra -DNDEBUG
NAME=LastEmperor.c
EXE=lastemperor
INSTALLDIR=/usr/bin/

# Targets

all:
	$(CC) $(CFLAGS) $(NAME) -o $(EXE)

strip:
	strip ./$(EXE)

clean:
	rm -f $(EXE)* *.out

install: all
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

bench: all
	./$(EXE) -bench

perft: all
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 11

.PHONY: all strip clean install bench perft
