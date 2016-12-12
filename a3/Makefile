CFLAGS = -Wall -g -std=c99 -Werror


BINARY = mapreduce
SRC = $(shell find *.c | sed '/test/d')
OBJ = ${SRC:.c=.o}

TEST_BINARY = test
TEST_SRC = $(shell find *.c | sed '/mapreduce/d')
TEST_OBJ = ${TEST_SRC:.c=.o}

.PHONY: $(BINARY) $(TEST_BINARY)

$(BINARY): clean $(OBJ)
	@gcc $(CFLAGS) -o $@ $(OBJ)
	@printf " → Linked %s \n" $@

%.o: %.c
	@gcc $(CFLAGS) -c $< -o $@
	@printf " ✔ Compiled %s \n" $@

clean:
	@rm -f $(BINARY) $(TEST_BINARY) *.o *.out

$(TEST_BINARY): CFLAGS += -DTESTING
$(TEST_BINARY): $(TEST_OBJ)
	@gcc $(CFLAGS) -o $@ $^ -lcunit
	@printf " → Linked %s \n" $@

check: clean $(TEST_BINARY)
	@valgrind -q ./test
