CC := gcc
CFLAGS := -I.
BIN := ./bin/

TARGETS := $(BIN)tcpserver $(BIN)tcpclient $(BIN)udpserver $(BIN)udpclient

.PHONY: all clean

all: $(TARGETS)

$(BIN):
	@mkdir -p $@

$(BIN)%: %.c | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN)
