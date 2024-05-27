#ifndef LCLEX_PARSER_H
#define LCLEX_PARSER_H

#include "tree.h"
#include <stdbool.h>

typedef enum {
    LCLEX_TOKEN_NULL, 
    LCLEX_TOKEN_IDENTIFIER, 
    LCLEX_TOKEN_INTLIT,
    LCLEX_TOKEN_LAMBDA,
    LCLEX_TOKEN_BROPEN, 
    LCLEX_TOKEN_BRCLOSE, 
    LCLEX_TOKEN_DOT,
    LCLEX_TOKEN_EOF
} lclex_tokentype_t;

typedef struct {
    char *data;
    lclex_tokentype_t type;
} lclex_token_t;

char *lclex_next_token(char **text, lclex_token_t *token);

char *lclex_type_string(lclex_tokentype_t type);

bool lclex_expect_token(lclex_token_t *token, lclex_tokentype_t expected);

lclex_node_t *lclex_parse_expression(char **text);

lclex_node_t *lclex_parse_application(char **text, lclex_token_t *token, 
                                      lclex_string_stack_t *stack);

lclex_node_t *lclex_parse_abstraction(char **text, lclex_token_t *token, 
                                      lclex_string_stack_t *stack);

lclex_node_t *lclex_parse_value(char **text, lclex_token_t *token, 
                                lclex_string_stack_t *stack);

#endif
