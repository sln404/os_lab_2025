CC := gcc
CFLAGS := -I. -pthread
BIN := ./bin/

TARGETS := $(BIN)server $(BIN)client

.PHONY: all clean

all: $(TARGETS)

$(BIN):
	@mkdir -p $@

$(BIN)%: %.c $(BIN)libmult_modulo.so | $(BIN)
	$(CC) $(CFLAGS) -L $(BIN) -lmult_modulo -Wl,-rpath,$(BIN) $^ -o $@

$(BIN)libmult_modulo.so: mult_modulo.c mult_modulo.h | $(BIN)
	$(CC) -shared $< -o $@

clean:
	rm -rf $(BIN)
