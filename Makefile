# Definitions (-fsanitize=undefined,alignment,bounds,shift)

CXX=clang++
CXXFLAGS=-march=native -O2 -Wall -pedantic -Wextra -DNDEBUG -DPEXT
FILES=lastemperor.cpp
EXE=lastemperor

# Targets

all:
	$(CXX) $(CXXFLAGS) $(FILES) -o $(EXE)

strip:
	strip ./$(EXE)

clean:
	rm -f $(EXE) *.out *.txt

# Unit testing

valgrind:
	g++ -Wall -O1 -ggdb3 $(FILES)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./a.out -bench 512

gprof:
	g++ -Wall -O1 -pg $(FILES)
	./a.out -bench 512 > /dev/null
	gprof --brief

.PHONY: all strip clean install valgrind gprof
