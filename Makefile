EXE = space

SRC = src
BUILD = build

EVALFILE = $(SRC)/2.nnue

COMPILER ?= g++

STD = -std=c++23
WARNINGS = -Wall -Wextra -Wshadow
OPTIMIZE = -O3 -flto

ARCH ?= native

ifeq ($(ARCH),native)
  ARCHFLAGS = -march=native
else ifeq ($(ARCH),x86-64)
  ARCHFLAGS = -march=x86-64
else ifeq ($(ARCH),x86-64-v3)
  ARCHFLAGS = -march=x86-64-v3
else
  $(error Unsupported ARCH "$(ARCH)". Supported: native, x86-64, x86-64-v3)
endif

CFLAGS = $(STD) -m64 $(OPTIMIZE) $(WARNINGS) $(ARCHFLAGS)

CPPFILES = \
	board.cpp \
	converts.cpp \
	evaluation.cpp \
	main.cpp \
	move.cpp \
	nnue.cpp \
	rt.cpp \
	search.cpp \
	tt.cpp \
	uci.cpp

OBJECTS = $(CPPFILES:.cpp=.o)
OBJECTS := $(addprefix $(BUILD)/,$(OBJECTS))

.PHONY: all clean help nnue

all: $(EXE)

help:
	@echo "Usage:"
	@echo "  make [-j] [ARCH=native|x86-64|x86-64-v3]"
	@echo
	@echo "Examples:"
	@echo "  make -j"
	@echo "  make -j ARCH=native"
	@echo "  make -j ARCH=x86-64"
	@echo "  make -j ARCH=x86-64-v3"

$(BUILD):
	mkdir -p $(BUILD)

nnue:
	python3 convert.py $(EVALFILE)

$(OBJECTS): nnue | $(BUILD)

$(EXE): $(OBJECTS)
	$(COMPILER) $(CFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(EXE)