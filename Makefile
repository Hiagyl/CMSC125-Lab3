CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread -g
# Add -fsanitize=thread to CFLAGS for Part 5

SRC = src/main.c src/bank.c src/timer.c src/transaction.c src/buffer_pool.c \
      src/lock_mgr.c src/metrics.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = bankdb

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

debug: CFLAGS += -fsanitize=thread
debug: clean $(TARGET)

test: $(TARGET)
	@echo "Running Trace Tests..."
	./$(TARGET) --accounts=tests/accounts.txt --trace=tests/trace_simple.txt
	./$(TARGET) --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt
	./$(TARGET) --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt
	./$(TARGET) --accounts=tests/accounts.txt --trace=tests/trace_abort.txt
	./$(TARGET) --accounts=tests/accounts.txt --trace=tests/trace_readers.txt

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)