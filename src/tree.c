#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

lclex_string_t const lclex_empty_string = { .external = NULL };

void lclex_init_string(lclex_string_t *string, char *data) {
    size_t len = strlen(data);
    string->external = malloc(len + 1);
    memcpy(string->external, data, len + 1);
}

lclex_string_t lclex_make_string(char *data) {
    lclex_string_t string;
    lclex_init_string(&string, data);

    return string;
}

void lclex_destruct_string(lclex_string_t string) {
    if (string.external != NULL) {
        free(string.external);
    }
}

void lclex_write_string(lclex_string_t string, FILE *stream) {
    fputs(string.external, stream);
}

char *lclex_get_raw_string(lclex_string_t *string, lclex_string_t *restore) {
    *restore = *string;

    return string->external;
}

void lclex_restore_string(lclex_string_t *string, lclex_string_t *restore) {
    *string = *restore;
}

bool lclex_equals_raw_string(lclex_string_t string, char *other) {
    lclex_string_t restore;
    char *raw = lclex_get_raw_string(&string, &restore);
    bool equal = strcmp(raw, other) == 0;
    lclex_restore_string(&restore, &string);
    
    return equal;
}


void lclex_init_string_stack(lclex_string_stack_t *stack) {
    stack->data = malloc(LCLEX_STRING_STACK_INIT_SIZE * sizeof(lclex_string_t));
    stack->size = 0;
    stack->cap = LCLEX_STRING_STACK_INIT_SIZE;
}

void lclex_destruct_string_stack(lclex_string_stack_t *stack) {
    free(stack->data);
}

void lclex_push_string(lclex_string_stack_t *stack, lclex_string_t string) {
    stack->data[stack->size] = string;
    stack->size++;
    if (stack->size >= stack->cap) {
        stack->data = realloc(stack->data, 2 * stack->cap);
        stack->cap *= 2;
    }
}

lclex_string_t lclex_pop_string(lclex_string_stack_t *stack) {
    if (stack->size == 0) {
        return lclex_empty_string;
    }
    stack->size--;
    return stack->data[stack->size];
}


lclex_node_t *lclex_new_node(lclex_type_t type, lclex_string_t data,
                             lclex_node_t *left, lclex_node_t *right) {
    lclex_node_t *node = malloc(sizeof(lclex_node_t));

    node->type = type;
    node->data = data;
    node->left = left;
    node->right = right;

    return node;
}

void lclex_free_node(lclex_node_t *node) {
    switch (node->type) {
        case LCLEX_APPLICATION:
            lclex_free_node(node->right);

            __attribute__((fallthrough));
        case LCLEX_ABSTRACTION:
            lclex_free_node(node->left);

            __attribute__((fallthrough));
        case LCLEX_FREE_VARIABLE:
            lclex_destruct_string(node->data);
            break;

        case LCLEX_BOUND_VARIABLE:
            break;
    }

    free(node);
}

void lclex_write_node(lclex_node_t *node, FILE *stream, 
                      lclex_string_stack_t *stack) {
    size_t i;
    lclex_string_t string;

    switch (node->type) {
        case LCLEX_APPLICATION:
            fprintf(stream, "(");
            lclex_write_node(node->left, stream, stack);
            fprintf(stream, " ");
            lclex_write_node(node->right, stream, stack);
            fprintf(stream, ")");
            break;

        case LCLEX_ABSTRACTION:
            lclex_push_string(stack, node->data);

            fprintf(stream, "(\\");
            lclex_write_string(node->data, stream);
            fprintf(stream, ".");
            lclex_write_node(node->left, stream, stack);
            fprintf(stream, ")");

            lclex_pop_string(stack);
            break;

        case LCLEX_FREE_VARIABLE:
            lclex_write_string(node->data, stream);
            break;

        case LCLEX_BOUND_VARIABLE:
            i = stack->size - node->data.index - 1;
            string = stack->data[i];

            if (string.internal == NULL) {
                fprintf(stream, "<%ld>", node->data.index);
            } else {
                lclex_write_string(string, stream);
            }
            break;
    }
}
