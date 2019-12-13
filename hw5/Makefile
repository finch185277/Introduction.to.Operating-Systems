CXX = clang++
ARG = -O3 -std=c++17 -g -o
RAW_FILES = main.cpp
EXEC_FILE = main

INPUT_FILE = input.txt
OUTPUT_FILES = output*.txt


all: $(RAW_FILES)
	$(CXX) $(ARG) $(EXEC_FILE) $^

.PHONY: test clean

test: all
	./$(EXEC_FILE) whois.txt
	./$(EXEC_FILE) zipf.txt

clean:
	rm -rf $(EXEC_FILE)
