CXX = clang++
ARG = -O3 -std=c++17 pkg-config fuse --cflags --libs
RAW_FILES = main.cpp
EXEC_FILE = main

INPUT_FILE = input.txt
OUTPUT_FILES = output*.txt


all: $(RAW_FILES)
	$(CXX) $(EXEC_FILE) $^  $(ARG)

.PHONY: test clean

test:
	./demo.sh $(EXEC_FILE)

clean:
	rm -rf $(EXEC_FILE)
