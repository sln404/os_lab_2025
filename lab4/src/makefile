CC := gcc
CFLAGS := -I.
BIN := ./bin/

OBJS    := $(BIN)utils.o $(BIN)find_min_max.o
TARGETS := $(BIN)parallel_min_max $(BIN)zombie $(BIN)process_memory $(BIN)parallel_sum

.PHONY: all clean

all: $(TARGETS)

$(BIN):
	@mkdir -p $@

$(BIN)parallel_min_max: parallel_min_max.c $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)zombie: zombie.c | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN)process_memory: process_memory.c | $(BIN)
	$(CC) $(CFLAGS) -w $^ -o $@

$(BIN)parallel_sum: parallel_sum.c $(BIN)utils.o $(BIN)libsum.so | $(BIN)
	$(CC) $(CFLAGS) -L $(BIN) -lsum -Wl,-rpath,$(BIN) -pthread $^ -o $@

$(BIN)libsum.so: sum.c sum.h | $(BIN)
	$(CC) -shared $< -o $@

$(BIN)%.o: %.c %.h | $(BIN)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN)
