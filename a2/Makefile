CFLAGS = -Wall -g -std=c99 -Werror
TEST = tests

friendme: friendme.o friends.o
	gcc $(CFLAGS) -o friendme friendme.o friends.o

friendme.o: friendme.c friends.h
	gcc $(CFLAGS) -c friendme.c

friends.o: friends.c friends.h
	gcc $(CFLAGS) -c friends.c

clean:
	rm -f friendme *.o

testCompile: CFLAGS += -DTESTING
testCompile: friends.o
	gcc $(CFLAGS) -c tests.c
	gcc $(CFLAGS) -o tests friends.o tests.o -lcunit

check: clean testCompile
	./tests
