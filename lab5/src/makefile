CC := gcc
CFLAGS := -I. -pthread
BIN := ./bin/

TARGETS := $(BIN)mutex $(BIN)no_mutex $(BIN)factorial $(BIN)deadlock

.PHONY: all clean

all: $(TARGETS)

$(BIN):
	@mkdir -p $@

$(BIN)no_mutex: mutex.c | $(BIN)
	$(CC) $(CFLAGS) -DNO_MUTEX $^ -o $@

$(BIN)%: %.c | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN)
