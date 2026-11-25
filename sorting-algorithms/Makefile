CC      ?= gcc
CFLAGS  ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
SRC     := src/sorting.c src/utils.c src/benchmark.c
BIN     := build/sorting

.PHONY: all run experiments test clean

all: $(BIN)

$(BIN): $(SRC) src/main.c src/*.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) src/main.c -o $@

run: $(BIN)
	./$(BIN)

experiments: $(BIN)
	./$(BIN) --experiments

build/test_sorting: tests/test_sorting.c src/sorting.c src/utils.c src/*.h
	@mkdir -p build
	$(CC) $(CFLAGS) tests/test_sorting.c src/sorting.c src/utils.c -o $@

test: build/test_sorting
	./build/test_sorting

clean:
	rm -rf build
