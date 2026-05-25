EXE=space

COMPILER=g++
VERSION=-std=c++23
CFLAGS=-m64 -O3 -mavx2 -mpopcnt -mlzcnt -mbmi2 -Wall -Wextra -Wshadow
CPPFILES=board.cpp converts.cpp evaluation.cpp main.cpp move.cpp rt.cpp search.cpp tt.cpp uci.cpp

OFILES=$(CPPFILES:.cpp=.o)

all: $(OFILES)
	$(COMPILER) $(VERSION) $(OFILES) -o $(EXE) $(CFLAGS)
	rm -f $(OFILES)

%.o: %.cpp
	$(COMPILER) $(VERSION) -c $< -o $@ $(CFLAGS)

$(OFILES): %.o: %.cpp
	$(COMPILER) $(VERSION) -c $< -o $@ $(CFLAGS)
