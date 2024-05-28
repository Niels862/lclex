#include "utils.h"
#include <stdlib.h>
#include <string.h>

void lclex_init_string_buf(lclex_string_buf_t *buf) {
    buf->str = malloc(LCLEX_STRING_BUF_INIT_SIZE);
    buf->len = 0;
    buf->cap = LCLEX_STRING_BUF_INIT_SIZE;
}

void lclex_destruct_string_buf(lclex_string_buf_t *buf) {
    free(buf->str);
}

void lclex_clear_string_buf(lclex_string_buf_t *buf) {
    buf->str[0] = '\0';
    buf->len = 0;
}

void lclex_readline(lclex_string_buf_t *buf, FILE *stream) {
    char *newline = NULL;
    
    lclex_clear_string_buf(buf);

    while (newline == NULL &&
           fgets(buf->str + buf->len, buf->cap - buf->len, stream) != NULL) {
        newline = strchr(buf->str, '\n');

        if (newline == NULL) {
            buf->str = realloc(buf->str, 2 * buf->cap);
            buf->cap *= 2;
        } else {
            *newline = '\0';
        }
        buf->len = strlen(buf->str);
    }
}

char *lclex_strdup(char *str) {
    size_t len = strlen(str);
    char *dup = malloc(len + 1);
    memcpy(dup, str, len + 1);

    return dup;
}

void lclex_init_stack(lclex_stack_t *stack) {
    stack->data = malloc(LCLEX_STACK_INIT_SIZE * sizeof(void *));
    stack->size = 0;
    stack->cap = LCLEX_STACK_INIT_SIZE;
}

void lclex_destruct_stack(lclex_stack_t *stack) {
    free(stack->data);
}

void lclex_clear_stack(lclex_stack_t *stack) {
    stack->size = 0;
}

void lclex_push_stack(lclex_stack_t *stack, void *data) {
    stack->data[stack->size] = data;
    stack->size++;
    if (stack->size >= stack->cap) {
        stack->data = realloc(stack->data, 2 * stack->cap * sizeof(void *));
        stack->cap *= 2;
    }
}

void *lclex_pop_stack(lclex_stack_t *stack) {
    if (stack->size == 0) {
        return NULL;
    }
    stack->size--;
    return stack->data[stack->size];
}
