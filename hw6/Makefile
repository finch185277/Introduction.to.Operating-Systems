CXX = g++
ARG = -O3 -std=c++17 `pkg-config fuse --cflags --libs`
RAW_FILES = main.cpp
EXEC_FILE = main

TAR_FILE = test.tar
OUT_FOLDER = output
MOUNT_POINT = tarfs

all: $(RAW_FILES)
	$(CXX) -o $(EXEC_FILE) $^ $(ARG)

.PHONY: test unmount clean

test: all
	./demo.sh $(EXEC_FILE)

umount:
	sudo umount -l $(MOUNT_POINT)

clean:
	rm -rf $(EXEC_FILE) $(TAR_FILE) $(OUT_FOLDER) ${MOUNT_POINT}
