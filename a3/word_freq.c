#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "mapreduce.h"


/*
 * Precondition: chunk is null-terminated.
 *
 * Write a sequence of Pairs to outfd, where the first element of the
 * pair is a word in the string, and the second element is 1.
 *
 * Note: the algorithm to remove spaces and punctuation will let the
 * empty string sneak through if a punctuation mark is surrounded by
 * white space.  It doesn't affect the results, so you don't need to fix it.
 */
void map(const char *chunk, int outfd) {
    Pair pair = {"", "1"};
    int index = 0;
    const char *cptr = chunk;

    // Get rid of any initial whitespace
    while (!isspace(*cptr)) {
        cptr++;
    }

    while (*cptr != '\0') {
        // If we have reached the end of the word then terminate and emit.
        if (isspace(*cptr)) {
            if (index == 0) { // don't emit empty strings.
                cptr++;
                continue;
            } else {
                pair.key[index] = '\0';
                write(outfd, &pair, sizeof(Pair));
                while (isspace(*cptr)) {
                    cptr++;
                }
                index = 0;
            }
        // ignore punctuation (This is a simplification.)
        } else if (ispunct(*cptr)) {
            cptr++;
        // otherwise add the character to our current word.
        } else {
            pair.key[index] = tolower(*cptr);
            cptr++;
            index++;
        }
    }
}


/* The key is a word, and the value is a list of key/value Pairs
 * that have this key. Each value is the count of the word
 * In the simple case the pairs in list will contain all ones as values.
 */
Pair reduce(const char *key, const LLValues *head_value) {
    const LLValues *curr = head_value;
    int result = 0;
    while (curr != NULL) {
        result += strtol(curr->value, NULL, 10);
        curr = curr->next;
    }
    char buf[MAX_VALUE];
    sprintf(buf, "%d", result);
    Pair pair;
    strncpy(pair.key, key, MAX_KEY);
    pair.key[MAX_KEY - 1] = '\0';
    strncpy(pair.value, buf, MAX_VALUE);
    pair.key[MAX_VALUE - 1] = '\0';
    return pair;
}
