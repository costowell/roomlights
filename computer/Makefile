CC = gcc

MAKEFLAGS := --jobs$(shell nproc)

SRC := src
OBJ := build
BIN := roomlights

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
HEADERS := $(wildcard $(SRC)/*.h)

LIBS    := libpulse-simple fftw3
CFLAGS  := $(shell pkg-config --cflags $(LIBS))
LDFLAGS := $(shell pkg-config --libs $(LIBS)) -lm -lpthread

debug: CFLAGS += -O0
debug: $(BIN)

release: CFLAGS += -O2
release: $(BIN)

$(BIN): $(OBJECTS) $(HEADERS)
	$(CC) -o $(BIN) $(OBJECTS) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ):
	mkdir -p $@

clean_debug: clean
	+@$(MAKE) --no-print-directory debug

clean_release: clean
	+@$(MAKE) --no-print-directory release

clean:
	-rm -rf $(OBJ)

.PHONY: clean debug release
