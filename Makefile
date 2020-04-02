# LastEmperor, a Chess960 move generator (Derived from Sapeli 1.67)
# GNU General Public License version 3; for details see LICENSE

#
# Definitions
#

# Compilers: [gcc, g++, clang, clang++]
CC=gcc

# Optimizations [-O2, -O3]
OPT=-O3 -march=native -fexpensive-optimizations

# Compile flags. [ -fsanitize=undefined ]
FLAGS=-Wall -pedantic -Wextra -Wshadow $(OPT) -DNDEBUG

FILES=LastEmperor.c
EXE=lastemperor
INSTALLDIR=/usr/bin/

#
# Public Targets
#

all:
	$(CC) $(FLAGS) $(FILES) -o $(EXE)

build-win64:
	x86_64-w64-mingw32-gcc -Wall -DNDEBUG -O2 LastEmperor.c -o lastemperor-win64.exe

release: build-profile build-win64

build-profile:
	$(CC) $(FLAGS) -fprofile-generate -lgcov $(FILES)
	./a.out -suite 4 > /dev/null
	rm -f a.out
	$(CC) $(FLAGS) $(FILES) -o $(EXE) -fprofile-use
	rm -f *.gcda a.out

# Avoid a conflict with a file of the same name, and improve performance.
.PHONY: all build-profile build-win64 strip clean release install id help	bench gprof valgrind

help:
	@echo "# Compiling LastEmperor #"
	@echo "Always '> lastemperor -bench' to see which settings are the best"
	@echo "> make target [debug=no]"
	@echo ""
	@echo "## Public Targets ##"
	@echo "help              This help"
	@echo "all               Build LastEmperor"
	@echo "build-profile     Build fast LastEmperor"
	@echo "install           Install LastEmperor"
	@echo "clean             Clean up"
	@echo "strip             Strip executable"
	@echo "id                Verify"
	@echo ""
	@echo "## Supported Compilers ##"
	@echo "gcc               GNU C Compiler"
	@echo "clang             Clang C Language Family Frontend for LLVM"
	@echo ""
	@echo "## Examples ##"
	@echo "Build             > make"
	@echo "Fast build        > make clean build-profile strip"
	@echo "Testing           > make clean all id bench"
	@echo "Build release     > make clean release"
	@echo "Install           > make clean build-profile strip install"

bench:
	./$(EXE) -bench

suite:
	./$(EXE) -suite 5

id:
	./$(EXE) -id

perft:
	./$(EXE) -fen "k7/8/2N5/1N6/8/8/8/K6n w - -" -perft 10

gprof:
	$(CC) $(FLAGS) -pg $(FILES) -o $(EXE)
	cp $(EXE) a.out
	./a.out -bench > /dev/null
	gprof

valgrind:
	$(CC) $(FLAGS) -ggdb3 $(FILES) -o $(EXE) && \
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./lastemperor -bench

install:
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

clean:
	rm -f $(EXE) a.out lastemperor* gmon.out valgrind-out.txt
