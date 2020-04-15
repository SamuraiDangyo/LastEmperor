# LastEmperor, a Chess960 movegen tool (Derived from Sapeli 1.67)
# Copyright (C) 2019 Toni Helminen
# GPLv3

# Compilers: [gcc, g++, clang, clang++]
CC=gcc
# -Weverything
OPT=-O3 -march=native
FLAGS=-Wall -pedantic -Wextra -Wshadow $(OPT) -DNDEBUG
FILES=LastEmperor.c
EXE=lastemperor
INSTALLDIR=/usr/bin/

all:
	$(CC) $(FLAGS) $(FILES) -o $(EXE)

build-win64:
	x86_64-w64-mingw32-gcc -Wall -DNDEBUG -O2 LastEmperor.c -o lastemperor$(VERSION)-win64.exe

build-profile:
	$(CC) $(FLAGS) -fprofile-generate -lgcov $(FILES)
	./a.out -bench > /dev/null
	rm -f a.out
	$(CC) $(FLAGS) $(FILES) -o $(EXE) -fprofile-use
	rm -f *.gcda a.out

release: build-profile build-win64

bench:
	./$(EXE) -bench

perft:
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 11

install:
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

clean:
	rm -f a.out lastemperor*

.PHONY: all build-profile build-win64 strip clean release install bench
