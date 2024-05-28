#ifndef LCLEX_UTILS_H
#define LCLEX_UTILS_H

#include <stddef.h>
#include <stdio.h>

#define LCLEX_STRING_BUF_INIT_SIZE 8

typedef struct {
    char *str;
    size_t len;
    size_t cap;
} lclex_string_buf_t;

void lclex_init_string_buf(lclex_string_buf_t *buf);

void lclex_destruct_string_buf(lclex_string_buf_t *buf);

void lclex_clear_string_buf(lclex_string_buf_t *buf);

void lclex_readline(lclex_string_buf_t *buf, FILE *stream);

char *lclex_strdup(char *str);

#define LCLEX_STACK_INIT_SIZE 32

typedef struct {
    void **data;
    size_t size;
    size_t cap;
} lclex_stack_t;

void lclex_init_stack(lclex_stack_t *stack);

void lclex_destruct_stack(lclex_stack_t *stack);

void lclex_clear_stack(lclex_stack_t *stack);

void lclex_push_stack(lclex_stack_t *stack, void *data);

void *lclex_pop_stack(lclex_stack_t *stack);

#endif
