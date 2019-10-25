CXX = clang++
ARG = -O3 -std=c++17 -g -o
RAW_FILES = main.cpp
EXEC_FILE = main
INPUT_FILE = input

all: $(RAW_FILES)
	$(CXX) $(ARG) $(EXEC_FILE) $^

.PHONY: test clean

test: all
	./$(EXEC_FILE) $(INPUT_FILE)

clean:
	rm -rf $(EXEC_FILE)
