#include "parser.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

char *lclex_next_token(lclex_parser_data_t *data) {
    static char replaced = '\0';

    char *p = *data->text;

    if (*p == '\0') {
        *p = replaced;
        replaced = '\0';
    }

    while (*p == ' ') {
        p++;
    }

    data->token->data = p;

    if (isalpha(*p) || *p == '_') {
        data->token->type = LCLEX_TOKEN_IDENTIFIER;
        do {
            p++;
        } while (isalpha(*p) || *p == '_' || isdigit(*p));
    }
    else if (isdigit(*p)) {
        data->token->type = LCLEX_TOKEN_INTLIT;
        do {
            p++;
        } while (isdigit(*p));
    }
    else if (*p == '\\' || *p == '(' || *p == ')' || *p == '.') {
        switch (*p) {
            case '\\':
                data->token->type = LCLEX_TOKEN_LAMBDA;
                break;
            case '(':
                data->token->type = LCLEX_TOKEN_BROPEN;
                break;
            case ')':
                data->token->type = LCLEX_TOKEN_BRCLOSE;
                break;
            case '.':
                data->token->type = LCLEX_TOKEN_DOT;
                break;
            default:
                break;
        }
        p++;
    }
    else if (*p == '\0') {
        data->token->type = LCLEX_TOKEN_EOF;
    } else {
        data->token->type = LCLEX_TOKEN_NULL;
        data->token->data = NULL;
    }

    replaced = *p;
    *p = '\0';
    *data->text = p;

    return data->token->data;
}

char *lclex_type_string(lclex_tokentype_t type) {
    static char *strings[] = {
        "null",
        "identifier",
        "intlit",
        "lambda",
        "bracket-open",
        "bracket-close",
        "dot",
        "eof"
    };

    return strings[type];
}

bool lclex_expect_token(lclex_token_t *token, lclex_tokentype_t expected) {
    if (token->type != expected) {
        fprintf(stderr, "Error: expected '%s', but got '%s'\n", 
                lclex_type_string(expected), 
                lclex_type_string(token->type));
        return false;
    }
    return true;
}

lclex_node_t *lclex_parse_expression(char **text, lclex_hashmap_t *defs) {
    lclex_stack_t stack;
    lclex_init_stack(&stack);
    
    lclex_token_t token;

    lclex_parser_data_t data = {
        .stack = &stack,
        .text = text,
        .token = &token,
        .defs = defs
    };

    lclex_next_token(&data);

    if (token.type == LCLEX_TOKEN_EOF) {
        fprintf(stderr, "Error: empty input string\n");
    }

    lclex_node_t *node = lclex_parse_application(&data);

    lclex_destruct_stack(data.stack);

    if (node == NULL || !lclex_expect_token(&token, LCLEX_TOKEN_EOF)) {
        if (node != NULL) {
            lclex_free_node(node);
        }
        return NULL;
    }

    return node;
}

lclex_node_t *lclex_parse_application(lclex_parser_data_t *data) {
    lclex_node_t *node = NULL; 
    
    while (data->token->type != LCLEX_TOKEN_EOF 
           && data->token->type != LCLEX_TOKEN_BRCLOSE) {
        lclex_node_t *sub = lclex_parse_abstraction(data);

        if (sub == NULL) {
            if (node != NULL) {
                lclex_free_node(node);
            }

            return NULL;
        }

        if (node == NULL) {
            node = sub;
        } else {
            node = lclex_new_node(LCLEX_APPLICATION, NULL, node, sub);
        }
    }

    return node;
}

lclex_node_t *lclex_parse_abstraction(lclex_parser_data_t *data) {    
    if (data->token->type != LCLEX_TOKEN_LAMBDA) {
        return lclex_parse_value(data);
    }

    lclex_next_token(data);

    if (!lclex_expect_token(data->token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    char *str = lclex_strdup(data->token->data);

    lclex_node_t *body;

    lclex_next_token(data);
    lclex_push_stack(data->stack, str);

    if (data->token->type == LCLEX_TOKEN_DOT) {
        lclex_next_token(data);
        body = lclex_parse_application(data);
    } else {
        body = lclex_parse_abstraction(data);
    }

    lclex_pop_stack(data->stack);

    if (body == NULL) {
        return NULL;
    }
    
    return lclex_new_node(LCLEX_ABSTRACTION, str, body, NULL);
}

lclex_node_t *lclex_parse_value(lclex_parser_data_t *data) {
    if (data->token->type == LCLEX_TOKEN_BROPEN) {
        lclex_next_token(data);

        lclex_node_t *node = lclex_parse_application(data);
        
        if (node == NULL) {
            return NULL;
        }

        if (!lclex_expect_token(data->token, LCLEX_TOKEN_BRCLOSE)) {
            return NULL;
        }

        lclex_next_token(data);

        return node;
    }
    
    if (data->token->type == LCLEX_TOKEN_INTLIT) {
        uint64_t n = strtoull(data->token->data, NULL, 10);
        
        lclex_next_token(data);

        return lclex_church_encode(n);
    }

    if (!lclex_expect_token(data->token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    lclex_type_t type;
    uintptr_t str;
    size_t i;

    for (i = 0; i < data->stack->size; i++) {
        char *bound_str = data->stack->data[data->stack->size - i - 1];

        if (strcmp(data->token->data, bound_str) == 0) {
            str = i;
            type = LCLEX_BOUND_VARIABLE;

            break;
        }
    }

    if (i == data->stack->size) {
        type = LCLEX_FREE_VARIABLE;
        str = (uintptr_t)lclex_strdup(data->token->data);
    }

    lclex_next_token(data);

    return lclex_new_node(type, (char *)str, NULL, NULL);
}
