# Definitions (-fsanitize=undefined,alignment,bounds,shift)

CC=clang++
CCFLAGS=-march=native -O2 -Wall -pedantic -Wextra -DNDEBUG -DMODERN -DPEXT
FILES=lastemperor.cpp
EXE=lastemperor
INSTALLDIR=/usr/bin/

# Targets

all:
	$(CC) $(CCFLAGS) $(FILES) -o $(EXE)

strip:
	strip ./$(EXE)

clean:
	rm -f $(EXE)* *.out *.txt

install: all
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

# Unit testing

valgrind:
	g++ -Wall -O1 -ggdb3 $(FILES)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./a.out -bench=0

gprof:
	g++ -Wall -O1 -pg $(FILES)
	./a.out -bench=0 > /dev/null
	gprof --brief

.PHONY: all strip clean install valgrind gprof
