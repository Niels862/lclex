#include "parser.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

char *lclex_next_token(lclex_token_t *token, char **text) {
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
    else if (*p == '\\' || *p == '(' || *p == ')' || *p == '.' || *p == '=') {
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

            case '=':
                token->type = LCLEX_TOKEN_EQUALS;
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
        "equals",
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

lclex_parser_signal_t lclex_parse_statement(char **text, lclex_hashmap_t *defs, 
                                            lclex_node_t **pnode) {
    lclex_parser_signal_t sig = LCLEX_PARSER_SUCCESS;

    lclex_token_t token;

    lclex_next_token(&token, text);

    char *key = NULL;

    if (strcmp(token.data, "def") == 0) {
        lclex_next_token(&token, text);
        if (lclex_expect_token(&token, LCLEX_TOKEN_IDENTIFIER)) {
            key = lclex_strdup(token.data);

            lclex_next_token(&token, text);
            if (lclex_expect_token(&token, LCLEX_TOKEN_EQUALS)) {
                lclex_next_token(&token, text);
            } else {
                sig = LCLEX_PARSER_FAILURE;
            }
        } else {
            sig = LCLEX_PARSER_FAILURE;
        }
    } else if (strcmp(token.data, "exit") == 0) {
        lclex_next_token(&token, text);
        sig = LCLEX_PARSER_EXIT;
    }

    lclex_node_t *node = NULL;
    
    if (sig == LCLEX_PARSER_SUCCESS) {
        lclex_stack_t stack;
        lclex_init_stack(&stack);

        lclex_parser_data_t parser = {
            .stack = &stack,
            .text = text,
            .token = &token,
            .defs = defs
        };

        node = lclex_parse_application(&parser);

        if (node == NULL) {
            sig = LCLEX_PARSER_FAILURE;
        }

        lclex_destruct_stack(parser.stack);
    }

    if (!lclex_expect_token(&token, LCLEX_TOKEN_EOF)) {
        sig = LCLEX_PARSER_FAILURE;
    }

    if (sig == LCLEX_PARSER_FAILURE) {
        if (node != NULL) {
            lclex_free_node(node);
        }
        if (key != NULL) {
            free(key);
        }

        *pnode = NULL;
    } else {
        if (key == NULL) {
            *pnode = node;
        } else {
            *pnode = NULL;
            lclex_insert_hashmap(defs, key, node);
        }
    }
    return sig;
}

lclex_node_t *lclex_parse_application(lclex_parser_data_t *parser) {
    lclex_node_t *node = NULL; 
    
    while (parser->token->type != LCLEX_TOKEN_EOF 
           && parser->token->type != LCLEX_TOKEN_BRCLOSE) {
        lclex_node_t *sub = lclex_parse_abstraction(parser);

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

lclex_node_t *lclex_parse_abstraction(lclex_parser_data_t *parser) {    
    if (parser->token->type != LCLEX_TOKEN_LAMBDA) {
        return lclex_parse_value(parser);
    }

    lclex_next_token(parser->token, parser->text);

    if (!lclex_expect_token(parser->token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    char *str = lclex_strdup(parser->token->data);

    lclex_node_t *body;

    lclex_next_token(parser->token, parser->text);
    lclex_push_stack(parser->stack, str);

    if (parser->token->type == LCLEX_TOKEN_DOT) {
        lclex_next_token(parser->token, parser->text);
        body = lclex_parse_application(parser);
    } else {
        body = lclex_parse_abstraction(parser);
    }

    lclex_pop_stack(parser->stack);

    if (body == NULL) {
        return NULL;
    }
    
    return lclex_new_node(LCLEX_ABSTRACTION, str, body, NULL);
}

lclex_node_t *lclex_parse_value(lclex_parser_data_t *parser) {
    lclex_node_t *node, *def_node;
    
    if (parser->token->type == LCLEX_TOKEN_BROPEN) {
        lclex_next_token(parser->token, parser->text);

        node = lclex_parse_application(parser);
        
        if (node == NULL) {
            return NULL;
        }

        if (!lclex_expect_token(parser->token, LCLEX_TOKEN_BRCLOSE)) {
            return NULL;
        }

        lclex_next_token(parser->token, parser->text);

        return node;
    }
    
    if (parser->token->type == LCLEX_TOKEN_INTLIT) {
        uint64_t n = strtoull(parser->token->data, NULL, 10);
        
        lclex_next_token(parser->token, parser->text);

        return lclex_church_encode(n);
    }

    if (!lclex_expect_token(parser->token, LCLEX_TOKEN_IDENTIFIER)) {
        return NULL;
    }

    for (size_t i = 0; i < parser->stack->size; i++) {
        char *bound_str = parser->stack->data[parser->stack->size - i - 1];

        if (strcmp(parser->token->data, bound_str) == 0) {
            lclex_next_token(parser->token, parser->text);

            return lclex_new_bound_variable(i);
        }
    }

    def_node = lclex_lookup_hashmap(parser->defs, parser->token->data);

    if (def_node == NULL) {
        node = lclex_new_free_variable(lclex_strdup(parser->token->data));
    } else {
        node = lclex_copy_node(def_node);
    }

    lclex_next_token(parser->token, parser->text);

    return node;
}
