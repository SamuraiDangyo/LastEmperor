##
# LastEmperor, a Chess960 move generator
# GNU General Public License version 3; for details see LICENSE
##

##
# ~~~ Compile Flags ~~~
#   -DNDEBUG          No debug. Much faster builds.
#   -DABSOLUTE_SCORE  Forces absolute scores. Default is relative.
##

##
# ~~~ Compilers ~~~
#   - gcc      Works best
#   - g++      Builds ok, lots of warnings
#   - clang    Builds ok, slower builds
#   - clang++  Builds ok, slower builds
##

##
# Definitions
##

# Compilers: gcc, g++, clang, clang++
CC=gcc

# Optimizations
OPT=-Ofast -march=native

# Compile flags. [ -fsanitize=undefined ]
FLAGS=-Wall -pedantic -Wextra -Wshadow $(OPT) -DNDEBUG

# Only LastEmperor.c
FILES=LastEmperor.c

# Executable
EXE=lastemperor

# Check whereis stockfish/etc and put LastEmperor there too
INSTALLDIR=/usr/games/

##
# Targets
##

# Standard build
build: $(FILES) clean
	@echo "> Building LastEmperor ..."
	$(CC) $(FLAGS) $(FILES) -o $(EXE)

# Builds for different architectures ...

# ARM Android build v7
buildarm: $(FILES) clean
	@echo "> Building LastEmperor for modern Android devices v7 ..."
	arm-linux-gnueabi-gcc -Wall -DNDEBUG -O2 -march=armv7-a LastEmperor.c -o lastemperor-arm

# ARM Android build v8
buildarm64: $(FILES) clean
	@echo "> Building LastEmperor for modern Android devices v8 ..."
	arm-linux-gnueabi-gcc -Wall -DNDEBUG -O2 -march=armv8-a LastEmperor.c -o lastemperor-arm64

# Windows 64 build
buildwin64: $(FILES) clean
	@echo "> Building LastEmperor for Windows 64 ..."
	x86_64-w64-mingw32-gcc -Wall -DNDEBUG -O2 LastEmperor.c -o lastemperorwin64.exe

# Windows 64 AMD build
buildwin64amd: $(FILES) clean
	@echo "> Building LastEmperor for Windows 64 AMD ..."
	x86_64-w64-mingw32-gcc -Wall -march=athlon64-sse3 -mpopcnt -DNDEBUG -O2 LastEmperor.c -o lastemperorwin64amd.exe

# The fastest build
buildprofile: $(FILES) clean
	@echo "> Profile building LastEmperor ..."
	$(CC) $(FLAGS) -fprofile-generate -lgcov $(FILES)
	@echo "> Benching ..."
	./a.out -suite 4 > /dev/null
	@echo "> Building optimized exe ..."
	rm -f a.out
	$(CC) $(FLAGS) $(FILES) -o $(EXE) -fprofile-use
	rm -f *.gcda a.out
	@echo "> Done!"

# Just run it
run: build
	./$(EXE)

# Verify that LastEmperor works
id: build
	./$(EXE) -id

# Perft suite
suite: build
	./$(EXE) -suite 3

# See help
help: build
	./$(EXE) -help

# Perft 
perft: build
	./$(EXE) -fen "8/2PPk3/8/8/7q/8/8/2K5 w - - 0 1" -perft 8

# gprof report
gprof: clean
	@echo "> Check w/ gprof ..."
	$(CC) $(FLAGS) -pg $(FILES) -o $(EXE)
	cp $(EXE) a.out
	@echo "> Benching ..."
	./a.out -bench > /dev/null
	@echo "> Done!"
	gprof

# Check memory profile
valgrind: clean
	@echo "> Check w/ valgrind ..."
	$(CC) $(FLAGS) -ggdb3 $(FILES) -o $(EXE) && \
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./lastemperor -bench
	@echo "> Done!"

# Install LastEmperor to your computer
install: buildprofile
	@echo You must be root to install
	if [ -d $(INSTALLDIR) ]; then sudo cp -f $(EXE) $(INSTALLDIR); fi

# Information of LastEmperor.c
info: clean
	@echo "> lines / words / bytes"
	wc LastEmperor.c

# Remove all unnecessary files
clean:
	rm -f $(EXE) a.out lastemperor-arm lastemperor-arm64 lastemperorwin64.exe lastemperorwin64amd.exe gmon.out games.pgn valgrind-out.txt LastEmperor-log.txt
