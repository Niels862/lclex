#ifndef LCLEX_PARSER_H
#define LCLEX_PARSER_H

#include "tree.h"
#include "hashmap.h"
#include <stdbool.h>

typedef enum {
    LCLEX_TOKEN_NULL, 
    LCLEX_TOKEN_IDENTIFIER, 
    LCLEX_TOKEN_INTLIT,
    LCLEX_TOKEN_LAMBDA,
    LCLEX_TOKEN_BROPEN, 
    LCLEX_TOKEN_BRCLOSE, 
    LCLEX_TOKEN_EQUALS,
    LCLEX_TOKEN_DOT,
    LCLEX_TOKEN_EOF
} lclex_tokentype_t;

typedef struct {
    char *data;
    lclex_tokentype_t type;
} lclex_token_t;

typedef struct {
    char **text;
    lclex_token_t *token;
    lclex_stack_t *stack;
    lclex_hashmap_t *defs;
} lclex_parser_data_t;

typedef enum {
    LCLEX_PARSER_FAILURE,
    LCLEX_PARSER_SUCCESS,
    LCLEX_PARSER_EXIT
} lclex_parser_signal_t;

char *lclex_next_token(lclex_token_t *token, char **text);

char *lclex_type_string(lclex_tokentype_t type);

bool lclex_expect_token(lclex_token_t *token, lclex_tokentype_t expected);

lclex_parser_signal_t lclex_parse_statement(char **text, lclex_hashmap_t *defs, 
                                             lclex_node_t **node);

lclex_node_t *lclex_parse_application(lclex_parser_data_t *data);

lclex_node_t *lclex_parse_abstraction(lclex_parser_data_t *data);

lclex_node_t *lclex_parse_value(lclex_parser_data_t *data);

#endif
