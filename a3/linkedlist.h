#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "mapreduce.h"

/*
 * Inserts into the list of keys and values pointed to by head_ptr.
 * Ensures that all values corresponding to a single key are grouped together.
 */
void insert_into_keys(LLKeyValues **head_ptr, Pair pair);

/*
 * Frees all memory associated with the given list of keys and values.
 */
void free_key_values_list(LLKeyValues *head);

#endif