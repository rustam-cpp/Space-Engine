EXE = space
EVALFILE = src/3.nnue

COMPILER = g++
VERSION = -std=c++23
CFLAGS = -m64 -O3 -march=native -flto -mavx2 -mpopcnt -mlzcnt -mbmi2 -Wall -Wextra -Wshadow

SRC = src
BUILD = build

CPPFILES = board.cpp converts.cpp evaluation.cpp main.cpp move.cpp nnue.cpp rt.cpp search.cpp tt.cpp uci.cpp

OBJECTS = $(CPPFILES:.cpp=.o)
OBJECTS := $(addprefix $(BUILD)/,$(OBJECTS))

all: builddir nnue $(EXE)

builddir:
	mkdir -p $(BUILD)

nnue: $(EVALFILE)
	python3 convert.py $(EVALFILE)

$(EXE): $(OBJECTS)
	$(COMPILER) $(VERSION) $(CFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	$(COMPILER) $(VERSION) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(EXE)