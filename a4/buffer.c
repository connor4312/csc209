#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Creates and returns a new string buffer. Henceforth the user will be
 * responsible for freeing the buffer using free_buffer.
 */
struct string_buffer *create_buffer()
{
    struct string_buffer *sb = malloc(sizeof(struct string_buffer));
    if (sb == NULL) {
        perror("malloc");
        exit(1);
    }

    sb->str = malloc(sizeof(char) * 32);
    if (sb->str == NULL) {
        perror("malloc");
        exit(1);
    }

    sb->str[0] = '\0';
    sb->len = 0;
    sb->size = 32;

    return sb;
}

/**
 * Frees resources associated with a string buffer.
 */
void buffer_free(struct string_buffer *sb)
{
    free(sb->str);
    free(sb);
}

/**
 * Appends the string str to the buffer, dynamically allocating space as
 * needed.
 */
void buffer_write(struct string_buffer *sb, const char *str)
{
    int input_len = strlen(str);

    while (sb->size - sb->len < input_len + 1) {
        int new_size = sb->size * 2;
        char *new_str = malloc(new_size * sizeof(char));
        if (new_str == NULL) {
            perror("malloc");
            exit(1);
        }

        memcpy(new_str, sb->str, sb->size);
        free(sb->str);

        sb->size = new_size;
        sb->str = new_str;
    }

    sb->len += input_len;
    strcat(sb->str, str);
}

/**
 * Allocates and copies the contents of the buffer to the target string.
 */
void buffer_copy_to(struct string_buffer *sb, char **str)
{
    char *out = malloc((sb->len + 1) * sizeof(char));
    if (out == NULL) {
        perror("malloc");
        exit(1);
    }

    memcpy(out, sb->str, (sb->len + 1) * sizeof(char));
    *str = out;
}

/**
 * Attempts to read a string up to the "needle" substring within the buffer.
 * If found, content before the substring will be stored in the target and
 * removed from the buffer.
 *
 * Returns 0 if the substring was found, or 1 otherwise. If 0 is returned,
 * the caller is responsible for freeing the target after they're done with it.
 */
int buffer_pull_before(struct string_buffer *sb, char **target, const char *needle)
{
    char *end = strstr(sb->str, needle);
    if (end == NULL) {
        return 1;
    }

    int index = end - sb->str;
    char *result = malloc(sizeof(char) * (index + 1));
    if (result == NULL) {
        perror("malloc");
        exit(1);
    }

    memcpy(result, sb->str, index);
    result[index] = '\0';
    *target = result;

    // Note: we use sb->len rather than sb->size here as we don't really care
    // about moving any memory we aren't using.
    int rest = sb->len + 1 - (index + strlen(needle));
    memmove(sb->str, end + strlen(needle), rest);
    sb->len -= index;

    return 0;
}
