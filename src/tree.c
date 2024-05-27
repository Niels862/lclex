#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

lclex_node_t *lclex_new_node(lclex_type_t type, char *data,
                             lclex_node_t *left, lclex_node_t *right) {
    lclex_node_t *node = malloc(sizeof(lclex_node_t));

    node->type = type;
    node->data.str = data;
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
            if (node->data.str != NULL) {
                free(node->data.str);
            }
            break;

        case LCLEX_BOUND_VARIABLE:
            break;
    }

    free(node);
}

void lclex_write_node(lclex_node_t *node, FILE *stream, 
                      lclex_stack_t *stack) {
    size_t i;
    char *str;

    switch (node->type) {
        case LCLEX_APPLICATION:
            fprintf(stream, "(");
            lclex_write_node(node->left, stream, stack);
            fprintf(stream, " ");
            lclex_write_node(node->right, stream, stack);
            fprintf(stream, ")");
            break;

        case LCLEX_ABSTRACTION:
            lclex_push_string(stack, node->data.str);

            fprintf(stream, "(\\");
            if (node->data.str != NULL) {
                fputs(node->data.str, stream);
            }
            fprintf(stream, ".");
            lclex_write_node(node->left, stream, stack);
            fprintf(stream, ")");

            lclex_pop_string(stack);
            break;

        case LCLEX_FREE_VARIABLE:
            fputs(node->data.str, stream);
            break;

        case LCLEX_BOUND_VARIABLE:
            i = stack->size - node->data.index - 1;
            str = stack->data[i];

            if (str == NULL) {
                fprintf(stream, "<%ld>", node->data.index);
            } else {
                fputs(str, stream);
            }
            break;
    }
}
