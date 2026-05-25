EXE = space

COMPILER = g++
VERSION = -std=c++23
CFLAGS = -m64 -O3 -mavx2 -mpopcnt -mlzcnt -mbmi2 -Wall -Wextra -Wshadow

SRC = src
BUILD = build

CPPFILES = board.cpp converts.cpp evaluation.cpp main.cpp move.cpp rt.cpp search.cpp tt.cpp uci.cpp

OBJECTS = $(CPPFILES:.cpp=.o)
OBJECTS := $(addprefix $(BUILD)/,$(OBJECTS))

all: builddir $(EXE)

builddir:
	mkdir -p $(BUILD)

$(EXE): $(OBJECTS)
	$(COMPILER) $(VERSION) $(CFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	$(COMPILER) $(VERSION) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(EXE)