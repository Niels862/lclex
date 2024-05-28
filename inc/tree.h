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

typedef uint64_t lclex_bruijn_index_t;

typedef struct lclex_node_t {
    lclex_type_t type;
    union {
        char *str;
        lclex_bruijn_index_t index;
    } data;
    struct lclex_node_t *left;
    struct lclex_node_t *right;
} lclex_node_t;

extern lclex_node_t *NULL_NODE;

lclex_node_t *lclex_new_node(lclex_type_t type, char *data,
                             lclex_node_t *left, lclex_node_t *right);

lclex_node_t *lclex_new_application(lclex_node_t *left, lclex_node_t *right);

lclex_node_t *lclex_new_abstraction(char *data, lclex_node_t *left);

lclex_node_t *lclex_new_free_variable(char *data);

lclex_node_t *lclex_new_bound_variable(lclex_bruijn_index_t index);

lclex_node_t *lclex_copy_node(lclex_node_t *node);

lclex_node_t *lclex_church_encode(uint64_t n);

uint64_t lclex_church_decode(lclex_node_t *node);

void lclex_free_node(void *data);

void lclex_free_partial_node(void *data);

void lclex_write_node_wrapped(lclex_node_t *node, FILE *stream, 
                              lclex_stack_t *stack);

void lclex_write_node(lclex_node_t *node, FILE *stream);

void lclex_remove_bound_names(lclex_node_t *node);

lclex_node_t **lclex_find_redex(lclex_node_t **pnode);

void lclex_find_bound_and_shift(lclex_node_t **pnode, lclex_node_t *new, 
                                lclex_bruijn_index_t index, 
                                lclex_stack_t *stack);

void lclex_shift(lclex_node_t *node, uint64_t shift, 
                 lclex_bruijn_index_t index);

void lclex_reduce_redex(lclex_node_t **redex, lclex_stack_t *stack);

void lclex_reduce_expression(lclex_node_t **pexpr, uint64_t max, 
                             bool show_reductions);

#endif
