# Definitions (-fsanitize=undefined,alignment,bounds,shift)

CC=clang++
CCFLAGS=-march=native -Ofast -Wall -pedantic -Wextra -DNDEBUG -DMODERN -DPEXT
NAME=lastemperor.cpp
EXE=lastemperor
INSTALLDIR=/usr/bin/

# Logic

ifeq (-DMODERN,$(findstring -DMODERN,$(CCFLAGS)))
  CCFLAGS+=-mpopcnt
endif

# Targets

all:
	$(CC) $(CCFLAGS) $(NAME) -o $(EXE)

strip:
	strip ./$(EXE)

clean:
	rm -f $(EXE)* *.out *.txt

install: all
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

# Unit testing

valgrind:
	g++ -Wall -O1 -ggdb3 $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./a.out -bench=0

gprof:
	g++ -Wall -O1 -pg $(NAME)
	./a.out -bench=0 > /dev/null
	gprof --brief

.PHONY: all strip clean install valgrind gprof
