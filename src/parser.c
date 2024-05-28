#include "parser.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>

char *lclex_next_token(char **text, lclex_token_t *token) {
    static char replaced = '\0';

    char *p = *text;

    if (*p == '\0') {
        *p = replaced;
        replaced = '\0';
    }

    while (*p == ' ') {
        p++;
    }

    token->data = p;

    if (isalpha(*p) || *p == '_') {
        token->type = LCLEX_TOKEN_IDENTIFIER;
        do {
            p++;
        } while (isalpha(*p) || *p == '_' || isdigit(*p));
    }
    else if (isdigit(*p)) {
        token->type = LCLEX_TOKEN_INTLIT;
        do {
            p++;
        } while (isdigit(*p));
    }
    else if (*p == '\\' || *p == '(' || *p == ')' || *p == '.') {
        switch (*p) {
            case '\\':
                token->type = LCLEX_TOKEN_LAMBDA;
                break;
            case '(':
                token->type = LCLEX_TOKEN_BROPEN;
                break;
            case ')':
                token->type = LCLEX_TOKEN_BRCLOSE;
                break;
            case '.':
                token->type = LCLEX_TOKEN_DOT;
                break;
            default:
                break;
        }
        p++;
    }
    else if (*p == '\0') {
        token->type = LCLEX_TOKEN_EOF;
    } else {
        token->type = LCLEX_TOKEN_NULL;
        token->data = NULL;
    }

    replaced = *p;
    *p = '\0';
    *text = p;

    return token->data;
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

lclex_node_t *lclex_parse_expression(char **text) {
    lclex_token_t token;
    lclex_next_token(text, &token);

    if (token.type == LCLEX_TOKEN_EOF) {
        fprintf(stderr, "Error: empty input string\n");
    }

    lclex_stack_t stack;
    lclex_init_stack(&stack);

    lclex_node_t *node = lclex_parse_application(text, &token, &stack);

    lclex_destruct_stack(&stack);

    if (node == NULL || !lclex_expect_token(&token, LCLEX_TOKEN_EOF)) {
        if (node != NULL) {
            lclex_free_node(node);
        }
        return NULL;
    }

    return node;
}

lclex_node_t *lclex_parse_application(char **text, lclex_token_t *token, 
                                      lclex_stack_t *stack) {
    lclex_node_t *node = NULL; 
    
    while (token->type != LCLEX_TOKEN_EOF 
           && token->type != LCLEX_TOKEN_BRCLOSE) {
        lclex_node_t *sub = lclex_parse_abstraction(text, token, stack);

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

lclex_node_t *lclex_parse_abstraction(char **text, lclex_token_t *token, 
                                      lclex_stack_t *stack) {    
    if (token->type != LCLEX_TOKEN_LAMBDA) {
        return lclex_parse_value(text, token, stack);
    }

    lclex_next_token(text, token);

    if (!lclex_expect_token(token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    char *str = lclex_strdup(token->data);

    lclex_node_t *body;

    lclex_next_token(text, token);
    lclex_push_stack(stack, str);

    if (token->type == LCLEX_TOKEN_DOT) {
        lclex_next_token(text, token);
        body = lclex_parse_application(text, token, stack);
    } else {
        body = lclex_parse_abstraction(text, token, stack);
    }

    lclex_pop_stack(stack);

    if (body == NULL) {
        return NULL;
    }
    
    return lclex_new_node(LCLEX_ABSTRACTION, str, body, NULL);
}

lclex_node_t *lclex_parse_value(char **text, lclex_token_t *token, 
                                lclex_stack_t *stack) {
    if (token->type == LCLEX_TOKEN_BROPEN) {
        lclex_next_token(text, token);

        lclex_node_t *node = lclex_parse_application(text, token, stack);
        
        if (node == NULL) {
            return NULL;
        }

        if (!lclex_expect_token(token, LCLEX_TOKEN_BRCLOSE)) {
            return NULL;
        }

        lclex_next_token(text, token);

        return node;
    }
    
    if (!lclex_expect_token(token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    lclex_type_t type;
    uintptr_t str;
    size_t i;

    for (i = 0; i < stack->size; i++) {
        char *bound_str = stack->data[stack->size - i - 1];

        if (strcmp(token->data, bound_str) == 0) {
            str = i;
            type = LCLEX_BOUND_VARIABLE;

            break;
        }
    }

    if (i == stack->size) {
        type = LCLEX_FREE_VARIABLE;
        str = (uintptr_t)lclex_strdup(token->data);
    }

    lclex_next_token(text, token);

    return lclex_new_node(type, (char *)str, NULL, NULL);
}
