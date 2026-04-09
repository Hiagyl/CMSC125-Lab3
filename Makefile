CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread -g
# Add -fsanitize=thread to CFLAGS for Part 5 of the lab to find race conditions

SRC = src/main.c src/bank.c src/timer.c src/transaction.c src/lock_mgr.c
OBJ = $(SRC:.c=.o)
TARGET = bankdb

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f src/*.o $(TARGET)