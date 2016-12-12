#include <stdio.h>

#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#define MAX_KEY 64       // Max size of key, including null-terminator.
#define MAX_VALUE 256    // Max size of value, including null-terminator.
#define MAX_FILENAME 32  // Max length of input file path, including null-terminator.
#define READSIZE 128     // Number of bytes to read per chunk of input file.
                         //   - You should allocate one more byte than this number
                         //     for a final null-terminator after these bytes.
#define PATH_DELIMITER '/' // Path separator, a forward slash on Linux

void map_worker(int outfd, int infd);
void reduce_worker(int outfd, int infd);

// A key-value pair emitted by a map function.
// All keys and values must be null-terminated.
typedef struct pair {
    char key[MAX_KEY];
    char value[MAX_VALUE];
} Pair;

// Linked list - each node contains a (string) value.
// value must be null-terminated.
typedef struct valuelist {
    char value[MAX_VALUE];
    struct valuelist *next;
} LLValues;

// Linked list - each node contains a unique key and list of corresponding values.
// key must be null-terminated.
typedef struct keyValues {
    char key[MAX_KEY];
    LLValues *head_value;
    struct keyValues *next;
} LLKeyValues;


/*
 * Takes a chunk of text and generates zero or more
 * Pair values, which it writes to outfd.
 *
 * Precondition: chunk is a null-terminated string.
 */
void map(const char *chunk, int outfd);

/*
 * Takes a key and list of values, and returns a new
 * Pair.
 *
 * Precondition: key and all strings in values are null-terminated.
 */
Pair reduce(const char *key, const LLValues *values);

/*
 * check_malloc works similarly to malloc, except that it prints an error
 * and exits if the allocation failed.
 */
void *checked_malloc(int size);

/*
 * check_fork works similarly to fork, except that it prints an error
 * and exits if the fork failed.
 */
int checked_fork();

/*
 * When this is called, we expect the forked process to have ended and
 * return a zero response code. If that isn't a case we error and close.
 */
void assert_fork_end(int pid, char *name);

/*
 * checked_open works similarly to fopen, except that it prints an error
 * and exits if the open failed.
 */
FILE *checked_fopen(char *file, char *mode);

/**
 * Joins the file paths `a` and `b` into one path, adding a path delimiter
 * if necessary. Target should have space for the sum of their lengths
 * plus 2 spaces for a potential delimiter and the null terminator.
 */
void path_join(char *target, char *a, char *b);

#endif
