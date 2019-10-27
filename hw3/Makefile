CXX = clang++
ARG = -O3 -std=c++17 -lpthread -g -o
RAW_FILES = main.cpp
EXEC_FILE = main

GEN_CPP = gen_input.cpp
GEN_EXE = gen_input

INPUT_SIZE = 1000000
INPUT_FILE = input
OUTPUT_FILES = output1.txt output2.txt


all: $(RAW_FILES)
	$(CXX) $(ARG) $(EXEC_FILE) $^

gen: $(GEN_CPP)
	$(CXX) $(ARG) $(GEN_EXE) $^
	./$(GEN_EXE) $(INPUT_FILE) $(INPUT_SIZE)

.PHONY: test clean

test: all gen
	./$(EXEC_FILE) $(INPUT_FILE)

clean:
	rm -rf $(EXEC_FILE) $(GEN_EXE) $(INPUT_FILE) $(OUTPUT_FILES)
