#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

/*
 * Return a pointer to a newly created LLKeyValues node.
 * Note it starts off with just a single value in its LLValues list.
 */
LLKeyValues *create_node(Pair pair) {
    LLKeyValues *new_node = malloc(sizeof(LLKeyValues));
    strncpy(new_node->key, pair.key, MAX_KEY - 1);
    new_node->key[MAX_KEY - 1] = '\0';
    new_node->head_value = malloc(sizeof(LLValues));
    strncpy(new_node->head_value->value, pair.value, MAX_VALUE - 1);
    new_node->head_value->value[MAX_VALUE - 1] = '\0';
    new_node->head_value->next = NULL;
    new_node->next = NULL;

    return new_node;
}

/*
 * Insert a value at the head of a Value list.
 */
void insert_value(LLKeyValues *list, const char *value) {
    LLValues *new_value = malloc(sizeof(LLValues));
    strncpy(new_value->value, value, MAX_VALUE - 1);
    new_value->value[MAX_VALUE - 1] = '\0';
    new_value->next = list->head_value;
    list->head_value = new_value;
}

/*
 * Insert into the list of keys and values pointed to by head_ptr.
 * Ensures that all values corresponding to a single key are grouped together.
 */
void insert_into_keys(LLKeyValues **head_ptr, Pair pair) {
    LLKeyValues *curr = *head_ptr;

    // Check if we need to create a new list of Pairs at the head of the list
    if (curr == NULL || strcmp(curr->key, pair.key) > 0) {
        LLKeyValues *new_node = create_node(pair);
        new_node->next = curr;
        *head_ptr = new_node;
    } else {
        LLKeyValues *prev = NULL;
        while (curr != NULL && strcmp(curr->key, pair.key) <= 0) {
            prev = curr;
            curr = curr->next;
        }

        if (prev == NULL) { // insert at head of list
            LLKeyValues *new_node = create_node(pair);
            new_node->next = curr;
            *head_ptr = new_node;
        } else if (strcmp(prev->key, pair.key) == 0) { // Key already exists
            insert_value(prev, pair.value);
        } else { // Need to insert new key
            LLKeyValues *new_node = create_node(pair);
            new_node->next = curr;
            prev->next = new_node;
        }
    }
}

/*
 * Free all memory associated with the given list of values.
 */
void free_value_list(LLValues *head) {
    LLValues *curr = head;
    while (curr != NULL) {
        LLValues *next = curr->next;
        free(curr);
        curr = next;
    }
}


/*
 * Free all memory associated with the given list of keys and values.
 */
void free_key_values_list(LLKeyValues *head) {
    LLKeyValues *curr = head;
    while (curr != NULL) {
        free_value_list(curr->head_value);
        LLKeyValues *next = curr->next;
        free(curr);
        curr = next;
    }
}
