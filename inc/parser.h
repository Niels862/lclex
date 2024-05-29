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

#define LCLEX_N_OPERATOR_LEVELS 8
#define LCLEX_MAX_OPERATORS_PER_LEVEL 8

typedef enum {
    LCLEX_OPERATOR_PREFIX,
    LCLEX_OPERATOR_INFIXR,
    LCLEX_OPERATOR_INFIXL,
    LCLEX_OPERATOR_SUFFIX
} lclex_operator_type_t;

typedef struct {
    char *operator;
    lclex_node_t *node;
} lclex_operator_def_t;

typedef struct {
    lclex_operator_type_t type;
    size_t n;
    lclex_operator_def_t defs[LCLEX_MAX_OPERATORS_PER_LEVEL];
} lclex_operator_level_t;

typedef struct {
    char **text;
    lclex_token_t *token;
    lclex_stack_t *stack;
    lclex_hashmap_t *defs;
    lclex_operator_level_t *opdefs;
} lclex_parser_data_t;

typedef enum {
    LCLEX_PARSER_FAILURE,
    LCLEX_PARSER_SUCCESS,
    LCLEX_PARSER_EXIT
} lclex_parser_signal_t;

bool lclex_is_idchar_start(char c);

bool lclex_is_idchar_continue(char c);

char *lclex_next_token(lclex_token_t *token, char **text);

char *lclex_type_string(lclex_tokentype_t type);

bool lclex_expect_token(lclex_token_t *token, lclex_tokentype_t expected);

lclex_node_t *lclex_new_binary_node(lclex_node_t *func, lclex_node_t *left, 
                                    lclex_node_t *right);

void lclex_init_operator_levels(lclex_operator_level_t *opdefs);

void lclex_destruct_operator_levels(lclex_operator_level_t *opdefs);

bool lclex_parse_definition(lclex_token_t *token, char **text, char **key);

bool lclex_parse_operator_definition(lclex_token_t *token, char **text, 
                                     char **key, size_t *level,
                                     lclex_operator_level_t opdefs[]);

lclex_parser_signal_t lclex_parse_statement(char **text, lclex_hashmap_t *defs, 
                                            lclex_operator_level_t opdefs[],
                                            lclex_node_t **pnode);

lclex_node_t *lclex_parse_application(lclex_parser_data_t *parser);

lclex_node_t *lclex_parse_abstraction(lclex_parser_data_t *parser);

lclex_node_t *lclex_parse_body(lclex_parser_data_t *parser, size_t level);

lclex_node_t *lclex_parse_value(lclex_parser_data_t *parser);

#endif
