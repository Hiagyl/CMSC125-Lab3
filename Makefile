CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread -g
# Add -fsanitize=thread to CFLAGS for Part 5

SRC = src/main.c src/bank.c src/timer.c src/transaction.c src/buffer_pool.c \
      src/lock_mgr.c src/metrics.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = tests/bankdb

# This is the important part:
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# This tells make HOW to create a .o file from a .c file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)