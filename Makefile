# LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
# GNU General Public License version 3; for details see LICENSE

# Compilers: [gcc, g++, clang, clang++]
CC=gcc

OPT=-O3 -march=native -fexpensive-optimizations
FLAGS=-Wall -pedantic -Wextra -Wshadow $(OPT) -DNDEBUG
FILES=LastEmperor.c
EXE=lastemperor
INSTALLDIR=/usr/bin/

all:
	$(CC) $(FLAGS) $(FILES) -o $(EXE)

build-win64:
	x86_64-w64-mingw32-gcc -Wall -DNDEBUG -O2 LastEmperor.c -o lastemperor-win64.exe

build-profile:
	$(CC) $(FLAGS) -fprofile-generate -lgcov $(FILES)
	./a.out -suite 4 > /dev/null
	rm -f a.out
	$(CC) $(FLAGS) $(FILES) -o $(EXE) -fprofile-use
	rm -f *.gcda a.out

release: build-profile build-win64

bench:
	./$(EXE) -bench

perft:
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 10

install:
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

clean:
	rm -f $(EXE) a.out lastemperor* gmon.out valgrind-out.txt

.PHONY: all build-profile build-win64 strip clean release install	bench
