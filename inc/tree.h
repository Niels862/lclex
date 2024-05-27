#ifndef LCLEX_TREE_H
#define LCLEX_TREE_H

#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LCLEX_APPLICATION, 
    LCLEX_ABSTRACTION, 
    LCLEX_FREE_VARIABLE, 
    LCLEX_BOUND_VARIABLE
} lclex_type_t;

typedef struct lclex_node_t {
    lclex_type_t type;
    union {
        char *str;
        uint64_t index;
    } data;
    struct lclex_node_t *left;
    struct lclex_node_t *right;
} lclex_node_t;

lclex_node_t *lclex_new_node(lclex_type_t type, char *data,
                             lclex_node_t *left, lclex_node_t *right);

void lclex_free_node(lclex_node_t *node);

void lclex_write_node(lclex_node_t *node, FILE *stream, 
                      lclex_stack_t *stack);

#endif
