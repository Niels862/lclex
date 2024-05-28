#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

lclex_node_t *NULL_NODE = NULL;

lclex_node_t *lclex_new_node(lclex_type_t type, char *data,
                             lclex_node_t *left, lclex_node_t *right) {
    lclex_node_t *node = malloc(sizeof(lclex_node_t));

    node->type = type;
    node->data.str = data;
    node->left = left;
    node->right = right;

    return node;
}

lclex_node_t *lclex_new_application(lclex_node_t *left, lclex_node_t *right) {
    return lclex_new_node(LCLEX_APPLICATION, NULL, left, right);
}

lclex_node_t *lclex_new_abstraction(char *data, lclex_node_t *left) {
    return lclex_new_node(LCLEX_ABSTRACTION, data, left, NULL);
}

lclex_node_t *lclex_new_free_variable(char *data) {
    return lclex_new_node(LCLEX_FREE_VARIABLE, data, NULL, NULL);
}

lclex_node_t *lclex_new_bound_variable(lclex_bruijn_index_t index) {
    return lclex_new_node(LCLEX_BOUND_VARIABLE, (char *)index, NULL, NULL);
}

lclex_node_t *lclex_copy_node(lclex_node_t *node) {
    lclex_node_t *left = NULL, *right = NULL;
    char *str = NULL;

    switch (node->type) {
        case LCLEX_APPLICATION:
            right = lclex_copy_node(node->right);

            __attribute__((fallthrough));
        case LCLEX_ABSTRACTION:
            left = lclex_copy_node(node->left);
        
            __attribute__((fallthrough));
        case LCLEX_FREE_VARIABLE:
            if (node->data.str != NULL) {
                str = lclex_strdup(node->data.str);
            }
            break;
            
        case LCLEX_BOUND_VARIABLE:
            str = (char *)(node->data.index);
            break;
    }

    return lclex_new_node(node->type, str, left, right);
}

lclex_node_t *lclex_church_encode(uint64_t n) {
    lclex_node_t *node = lclex_new_bound_variable(0);

    for (uint64_t i = n; i > 0; i--) {
        lclex_node_t *f = lclex_new_bound_variable(1);
        node = lclex_new_application(f, node);
    }

    lclex_node_t *abstr_x = lclex_new_abstraction(lclex_strdup("x"), node);
    lclex_node_t *abstr_f = lclex_new_abstraction(lclex_strdup("f"), abstr_x);
    
    return abstr_f;
}

uint64_t lclex_church_decode(lclex_node_t *node) {
    lclex_node_t *temp = node;
    if (temp->type != LCLEX_ABSTRACTION) {
        return UINT64_MAX;
    }

    temp = temp->left;
    if (temp->type != LCLEX_ABSTRACTION) {
        return UINT64_MAX;
    }

    uint64_t n = 0;
    temp = temp->left;
    while (temp->type != LCLEX_BOUND_VARIABLE) {
        if (temp->type != LCLEX_APPLICATION 
            || temp->left->type != LCLEX_BOUND_VARIABLE
            || temp->left->data.index != 1) {
            return UINT64_MAX;
        }

        temp = temp->right;
        n++;
    }

    if (temp->data.index != 0) {
        return UINT64_MAX;
    }
    return n;
}

void lclex_free_node(void *data) {
    lclex_node_t *node = data;
    
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

void lclex_free_partial_node(void *data) {
    lclex_node_t *node = data;
    
    if (node->left != NULL) {
        lclex_free_partial_node(node->left);
    }

    if (node->right != NULL) {
        lclex_free_partial_node(node->right);
    }

    if (node->data.str != NULL && node->type != LCLEX_BOUND_VARIABLE) {
        free(node->data.str);
    }

    free(node);
}

void lclex_write_node_wrapped(lclex_node_t *node, FILE *stream, 
                              lclex_stack_t *stack) {
    size_t i;
    char *str;

    switch (node->type) {
        case LCLEX_APPLICATION:
            fprintf(stream, "(");
            lclex_write_node_wrapped(node->left, stream, stack);
            fprintf(stream, " ");
            lclex_write_node_wrapped(node->right, stream, stack);
            fprintf(stream, ")");
            break;

        case LCLEX_ABSTRACTION:
            lclex_push_stack(stack, node->data.str);

            fprintf(stream, "(\\");
            if (node->data.str != NULL) {
                fputs(node->data.str, stream);
            }
            fprintf(stream, ".");
            lclex_write_node_wrapped(node->left, stream, stack);
            fprintf(stream, ")");

            lclex_pop_stack(stack);
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

void lclex_write_node(lclex_node_t *node, FILE *stream) {
    lclex_stack_t stack;
    lclex_init_stack(&stack);

    lclex_write_node_wrapped(node, stream, &stack);
    printf("\n");

    lclex_destruct_stack(&stack);
}

void lclex_remove_bound_names(lclex_node_t *node) {
    switch (node->type) {
        case LCLEX_APPLICATION:
            lclex_remove_bound_names(node->right);

            __attribute__((fallthrough));
        case LCLEX_ABSTRACTION:
            lclex_remove_bound_names(node->left);
            
            if (node->data.str != NULL) {
                free(node->data.str);
                node->data.str = NULL;
            }

            break;

        case LCLEX_FREE_VARIABLE:
        case LCLEX_BOUND_VARIABLE:
            break;
    }
}

lclex_node_t **lclex_find_redex(lclex_node_t **pnode) {
    lclex_node_t **redex;
    lclex_node_t *node = *pnode;

    switch (node->type) {
        case LCLEX_APPLICATION:
            if (node->left->type == LCLEX_ABSTRACTION) {
                return pnode;
            }

            redex = lclex_find_redex(&node->left);
            if (*redex != NULL_NODE) {
                return redex;
            }
            return lclex_find_redex(&node->right);

        case LCLEX_ABSTRACTION:
            return lclex_find_redex(&node->left);
        
        case LCLEX_FREE_VARIABLE:
        case LCLEX_BOUND_VARIABLE:
            return &NULL_NODE;
    }

    return &NULL_NODE;
}

void lclex_find_bound_and_shift(lclex_node_t **pnode, lclex_node_t *new, 
                                lclex_bruijn_index_t index, 
                                lclex_stack_t *stack) {
    lclex_node_t *node = *pnode;

    switch (node->type) {
        case LCLEX_APPLICATION:
            lclex_find_bound_and_shift(&node->left, new, index, stack);
            lclex_find_bound_and_shift(&node->right, new, index, stack);
            break;

        case LCLEX_ABSTRACTION:
            lclex_find_bound_and_shift(&node->left, new, index + 1, stack);
            break;
        
        case LCLEX_FREE_VARIABLE:
            break;
        
        case LCLEX_BOUND_VARIABLE:
            if (node->data.index == index) {
                lclex_push_stack(stack, pnode);
                lclex_push_stack(stack, (void *)(index));
            } else if (node->data.index > index) {
                node->data.index--;
            }
            break;
    }
}

void lclex_shift(lclex_node_t *node, uint64_t shift) {
    switch (node->type) {
        case LCLEX_APPLICATION:
            lclex_shift(node->right, shift);
        
            __attribute__((fallthrough));
        case LCLEX_ABSTRACTION:
            lclex_shift(node->left, shift);
            break;

        case LCLEX_FREE_VARIABLE:
            break;

        case LCLEX_BOUND_VARIABLE:
            node->data.index += shift;
            break;
    }
}

void lclex_reduce_redex(lclex_node_t **redex, lclex_stack_t *stack) {
    lclex_node_t *node = *redex;
    lclex_node_t *abstr = node->left;

    lclex_clear_stack(stack);
    lclex_find_bound_and_shift(&abstr->left, node->right, 0, stack);

    for (size_t i = 0; i < stack->size; i += 2) {
        lclex_node_t **ptarget = stack->data[i];
        lclex_bruijn_index_t index = (lclex_bruijn_index_t)(stack->data[i + 1]);
        lclex_free_node(*ptarget);

        if (i == stack->size) {
            *ptarget = node->right;
            node->right = NULL;
        } else {
            *ptarget = lclex_copy_node(node->right);
        }

        lclex_shift(*ptarget, index);
    }
    *redex = abstr->left;

    abstr->left = NULL;
    lclex_free_partial_node(node);
}

void lclex_reduce_expression(lclex_node_t **pexpr, uint64_t max) {
    lclex_node_t **redex;
    uint64_t count = 0;

    lclex_stack_t stack;
    lclex_init_stack(&stack);

    while (count < max && *(redex = lclex_find_redex(pexpr)) != NULL_NODE) {
        lclex_reduce_redex(redex, &stack);
        count++;

        printf("%ld: ", count);
        lclex_write_node(*pexpr, stdout);
    }

    lclex_destruct_stack(&stack);
}
