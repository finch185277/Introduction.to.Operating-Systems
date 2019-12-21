CXX = gcc
ARG = -O3 -std=c++17 `pkg-config fuse --cflags --libs`
RAW_FILES = main.cpp
EXEC_FILE = main

INPUT_FILE = input.txt
OUTPUT_FILES = output*.txt


all: $(RAW_FILES)
	$(CXX) -o $(EXEC_FILE) $^ $(ARG)

.PHONY: test clean

test: all
	./demo.sh $(EXEC_FILE)

clean:
	rm -rf $(EXEC_FILE)
