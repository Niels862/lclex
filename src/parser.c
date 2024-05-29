#include "parser.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

bool lclex_is_idchar_start(char c) {
    return isalpha(c) || c == '_' || c == '+' || c == '-' || c == '*' 
           || c == '/' || c == '%' || c == '^' || c == '&' || c == '#'
           || c == '|' || c == '~' || c == '?' || c == '<' || c == '>';
}

bool lclex_is_idchar_continue(char c) {
    return isdigit(c) || lclex_is_idchar_start(c);
}

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

    if (lclex_is_idchar_start(*p)) {
        token->type = LCLEX_TOKEN_IDENTIFIER;
        do {
            p++;
        } while (lclex_is_idchar_continue(*p));
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

lclex_node_t *lclex_new_binary_node(lclex_node_t *func, lclex_node_t *left, 
                                    lclex_node_t *right) {
    lclex_node_t *node1 = lclex_copy_node(func);
    lclex_node_t *node2 = lclex_new_application(node1, left);
    return lclex_new_application(node2, right);
}

void lclex_init_operator_levels(lclex_operator_level_t *opdefs) {
    for (size_t i = 0; i < LCLEX_N_OPERATOR_LEVELS; i++) {
        opdefs[i].n = 0;
        switch (i) {
            case 0: 
                opdefs[i].type = LCLEX_OPERATOR_SUFFIX;
                break;

            case 1:
                opdefs[i].type = LCLEX_OPERATOR_PREFIX;
                break;

            case 3:
                opdefs[i].type = LCLEX_OPERATOR_INFIXR;
                break;
            
            default:
                opdefs[i].type = LCLEX_OPERATOR_INFIXL;
                break;
        }
    }
}

void lclex_destruct_operator_levels(lclex_operator_level_t *opdefs) {
    for (size_t i = 0; i < LCLEX_N_OPERATOR_LEVELS; i++) {
        lclex_operator_level_t *level_def = &opdefs[i];
        for (size_t j = 0; j < level_def->n; j++) {
            free(level_def->defs[j].operator);
            lclex_free_node(level_def->defs[j].node);
        }
    }
}

bool lclex_parse_definition(lclex_token_t *token, char **text, char **key) {
    if (!lclex_expect_token(token, LCLEX_TOKEN_IDENTIFIER)) {
        return false;
    }
    *key = lclex_strdup(token->data);

    lclex_next_token(token, text);
    if (!lclex_expect_token(token, LCLEX_TOKEN_EQUALS)) {
        return false;
    }

    lclex_next_token(token, text);

    return true;
}

bool lclex_parse_operator_definition(lclex_token_t *token, char **text, 
                                     char **key, size_t *level,
                                     lclex_operator_level_t opdefs[]) {
    if (!lclex_expect_token(token, LCLEX_TOKEN_IDENTIFIER)) {
        return false;
    }
    *key = lclex_strdup(token->data);

    lclex_next_token(token, text);
    if (!lclex_expect_token(token, LCLEX_TOKEN_INTLIT)) {
        return false;
    }
    *level = strtoul(token->data, NULL, 10);
    if (*level < 1 || *level > LCLEX_N_OPERATOR_LEVELS) {
        fprintf(stderr, "Error: operator level not in range\n");
        return false;
    }
    if (opdefs[*level].n == LCLEX_MAX_OPERATORS_PER_LEVEL) {
        fprintf(stderr, "Error: operator level is full\n");
        return false;
    }

    lclex_next_token(token, text);
    if (!lclex_expect_token(token, LCLEX_TOKEN_EQUALS)) {
        return false;
    }

    lclex_next_token(token, text);

    return true;
}


lclex_parser_signal_t lclex_parse_statement(char **text, lclex_hashmap_t *defs, 
                                            lclex_operator_level_t opdefs[],
                                            lclex_node_t **pnode) {
    lclex_parser_signal_t sig = LCLEX_PARSER_SUCCESS;

    lclex_token_t token;

    lclex_next_token(&token, text);

    char *key = NULL;
    size_t level = 0;

    if (strcmp(token.data, "def") == 0) {
        lclex_next_token(&token, text);
        if (!lclex_parse_definition(&token, text, &key)) {
            sig = LCLEX_PARSER_FAILURE;
        }
    } else if (strcmp(token.data, "opdef") == 0) {
        lclex_next_token(&token, text);
        if (!lclex_parse_operator_definition(&token, text, &key, 
                                             &level, opdefs)) {
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
            .defs = defs,
            .opdefs = opdefs
        };

        node = lclex_parse_application(&parser);

        if (node == NULL) {
            sig = LCLEX_PARSER_FAILURE;
        }

        lclex_destruct_stack(parser.stack);
    }

    if (sig != LCLEX_PARSER_FAILURE 
        && !lclex_expect_token(&token, LCLEX_TOKEN_EOF)) {
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
            if (level == 0) {
                lclex_insert_hashmap(defs, key, node);
            } else {
                lclex_operator_level_t *level_def = &opdefs[level - 1];

                level_def->defs[level_def->n].operator = key;
                level_def->defs[level_def->n].node = node;
                level_def->n++;
            }
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

    if (node == NULL) {
        printf("Error: expected value\n");
    }

    return node;
}

lclex_node_t *lclex_parse_abstraction(lclex_parser_data_t *parser) {    
    if (parser->token->type != LCLEX_TOKEN_LAMBDA) {
        return lclex_parse_body(parser, LCLEX_N_OPERATOR_LEVELS);
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

lclex_node_t *lclex_parse_body(lclex_parser_data_t *parser, size_t level) {
    if (level == 0) {
        return lclex_parse_value(parser);
    }

    lclex_operator_level_t *level_def = &parser->opdefs[level - 1];

    lclex_node_t *node = lclex_parse_body(parser, level - 1);
    if (node == NULL) {
        return NULL;
    }

    lclex_node_t *other = NULL;
    bool found = true;

    while (found) {
        found = false;

        for (size_t i = 0; i < level_def->n && !found; i++) {
            if (strcmp(level_def->defs[i].operator, 
                       parser->token->data) == 0) {
                found = true;
                lclex_next_token(parser->token, parser->text);

                switch (level_def->type) {
                    case LCLEX_OPERATOR_PREFIX:
                        break;

                    case LCLEX_OPERATOR_INFIXR:
                        other = lclex_parse_body(parser, level);
                        if (other == NULL) {
                            free(node);
                        }
                        return lclex_new_binary_node(level_def->defs[i].node, 
                                                     node, other);

                    case LCLEX_OPERATOR_INFIXL:
                        other = lclex_parse_body(parser, level - 1);
                        if (other == NULL) {
                            free(node);
                        }
                        node = lclex_new_binary_node(level_def->defs[i].node, 
                                                     node, other);
                        break;
                    
                    case LCLEX_OPERATOR_SUFFIX:
                        break;
                }
            }
        }
    }

    return node;
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
