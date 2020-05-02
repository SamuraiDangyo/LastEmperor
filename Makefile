# Copyright (C) 2019-2020 Toni Helminen <GPLv3>

CC=gcc
INSTALLDIR=/usr/bin/
EXE=lastemperor

all:
	$(CC) LastEmperor.c -Wall -pedantic -Wextra -Wshadow -O2 -march=native $(EXTRAFLAGS) -o $(EXE)

bench:
	./$(EXE) -bench

perft:
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 11

install:
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

clean:
	rm -f $(EXE)*

.PHONY: all strip clean install bench
