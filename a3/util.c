#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mapreduce.h"


void *checked_malloc(int size)
{
    void *pos = malloc(size);
    if (pos == NULL) {
        perror("malloc");
        exit(1);
    }

    return pos;
}

int checked_fork()
{
    int out = fork();
    if (out == -1) {
        perror("fork");
        exit(1);
    }

    return out;
}

FILE *checked_fopen(char *file, char *mode)
{
    FILE *fp = fopen(file, mode);
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    return fp;
}

void assert_fork_end(int pid, char *name)
{
    int status;
    while (waitpid(pid, &status, 0) > -1);

    if (!WIFEXITED(status)) {
        printf("Expected `%s` to have exited.\n", name);
        exit(1);
    }

    int code = WEXITSTATUS(status);
    if (code > 0) {
        printf("Expected `%s` to have exited with status code 0, got %d.\n", name, code);
        exit(code);
    }
}

void path_join(char *target, char *a, char *b)
{
    strcpy(target, a);

    int len = strlen(a);
    if (target[len - 1] != PATH_DELIMITER && b[0] != PATH_DELIMITER) {
        target[len] = PATH_DELIMITER;
        target[len + 1] = '\0';
    }

    strcat(target, b);
}
