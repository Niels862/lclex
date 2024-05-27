#ifndef LCLEX_TREE_H
#define LCLEX_TREE_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define LCLEX_STRING_STACK_INIT_SIZE 32

typedef enum {
    LCLEX_APPLICATION, 
    LCLEX_ABSTRACTION, 
    LCLEX_FREE_VARIABLE, 
    LCLEX_BOUND_VARIABLE
} lclex_type_t;

typedef union {
    char internal[8];
    char *external;
    uint64_t index;
} lclex_string_t;

typedef struct lclex_node_t {
    lclex_type_t type;
    lclex_string_t data;
    struct lclex_node_t *left;
    struct lclex_node_t *right;
} lclex_node_t;

typedef struct {
    lclex_string_t *data;
    size_t size;
    size_t cap;
} lclex_string_stack_t;


extern lclex_string_t const lclex_empty_string;

void lclex_init_string(lclex_string_t *string, char *data);

lclex_string_t lclex_make_string(char *data);

void lclex_destruct_string(lclex_string_t string);

void lclex_write_string(lclex_string_t string, FILE *stream);

/* Modifies original. Should always be followed by restore_string when
   data is no longer used. */
char *lclex_get_raw_string(lclex_string_t *string, lclex_string_t *restore);

void lclex_restore_string(lclex_string_t *string, lclex_string_t *restore);

bool lclex_equals_raw_string(lclex_string_t string, char *other);

void lclex_init_string_stack(lclex_string_stack_t *stack);

void lclex_destruct_string_stack(lclex_string_stack_t *stack);

void lclex_push_string(lclex_string_stack_t *stack, lclex_string_t string);

lclex_string_t lclex_pop_string(lclex_string_stack_t *stack);


lclex_node_t *lclex_new_node(lclex_type_t type, lclex_string_t data,
                             lclex_node_t *left, lclex_node_t *right);

void lclex_free_node(lclex_node_t *node);

void lclex_write_node(lclex_node_t *node, FILE *stream, 
                      lclex_string_stack_t *stack);

#endif
