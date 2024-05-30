#define _GNU_SOURCE

#include "tree.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct {
    bool show_numbers;
    bool show_reductions;
    bool show_parsed;
    bool hide_results;
} lclex_options_t;

void lclex_help(char *argv[]) {
    fprintf(stderr, "Usage: %s [-nrph]\n", argv[0]);
    fprintf(stderr, "    -n: show numbers\n");
    fprintf(stderr, "    -r: show reductions\n");
    fprintf(stderr, "    -p: show parsed expression\n");
    fprintf(stderr, "    -h: hide result expression\n");
}

char *std_exprs[] = {
    "def id = \\x.x",
    "def add = \\m.\\n.\\f.\\x.m f (n f x)",
    "def succ = add 1",
    "def mul = \\m.\\n.\\f.\\x.m (n f) x",
    "def exp = \\m.\\n.n m",
    "def square = \\m.exp m 2",
    "def pred = \\n.\\f.\\x.n (\\g.\\h. h (g f)) (\\u x) id",
    "def sub = \\m.\\n.(n pred) m",
    "def div = (\\n.((\\f.(\\x.x x) (\\x.f (x x)))" \
    "(\\c.\\n.\\m.\\f.\\x.(\\d.(\\n.n (\\x.(\\a.\\b.b)) (\\a.\\b.a))" \
    "d ((\\f.\\x.x) f x) (f (c d m f x))) ((\\m.\\n.n" \
    "(\\n.\\f.\\x.n (\\g.\\h.h (g f)) (\\u.x) (\\u.u)) m) n m)))" \
    "((\\n.\\f.\\x. f (n f x)) n))",
    "opdef ^ 3 = exp",
    "opdef * 4 = mul",
    "opdef / 4 = div",
    "opdef + 5 = add",
    "opdef - 5 = sub"
};

int main(int argc, char *argv[]) {
    lclex_options_t opts = {
        .show_numbers = false,
        .show_reductions = false,
        .hide_results = false
    };

    int opt;
    while ((opt = getopt(argc, argv, "nrph")) != -1) {
        switch (opt) {
            case 'n':
                opts.show_numbers = true;
                break;
            
            case 'r':
                opts.show_reductions = true;
                break;
            
            case 'p':
                opts.show_parsed = true;
                break;

            case 'h':
                opts.hide_results = true;
                break;
            
            case '?':
                lclex_help(argv);

            __attribute__((fallthrough));
            default:
                return 1;
        }
    }

    lclex_string_buf_t buf;
    lclex_init_string_buf(&buf);

    lclex_hashmap_t defs;
    lclex_init_string_hashmap(&defs, lclex_free_node);

    lclex_operator_level_t opdefs[LCLEX_N_OPERATOR_LEVELS];
    lclex_init_operator_levels(opdefs);

    lclex_parser_signal_t sig = LCLEX_PARSER_SUCCESS;

    for (size_t i = 0; i < sizeof(std_exprs) / sizeof(*std_exprs); i++) {
        char *command = lclex_strdup(std_exprs[i]);
        char *text = command;
        lclex_node_t *expr;

        sig = lclex_parse_statement(&text, &defs, opdefs, &expr);

        if (sig == LCLEX_PARSER_FAILURE) {
            fprintf(stderr, "Error: syntax error in standard expression\n");

            sig = LCLEX_PARSER_EXIT;
            break;
        }

        if (expr != NULL) {
            lclex_free_node(expr);
        }

        free(command);
    }

    while (sig != LCLEX_PARSER_EXIT) {
        printf(">>> ");
        lclex_readline(&buf, stdin);

        char *text = buf.str;
        lclex_node_t *expr;

        sig = lclex_parse_statement(&text, &defs, opdefs, &expr);

        if (expr == NULL) {
            continue;
        }

        if (opts.show_parsed) {
            lclex_write_node(expr, stdout);
        }

        lclex_reduce_expression(&expr, UINT64_MAX, opts.show_reductions);
        
        if (!opts.hide_results) {
            printf("> ");
            lclex_write_node(expr, stdout);
        }
        
        if (opts.show_numbers) {
            uint64_t n = lclex_church_decode(expr);
            if (n == UINT64_MAX) {
                printf("> nan\n");
            } else {
                printf("> %ld\n", n);
            }
        } 
        
        lclex_free_node(expr);
    }

    lclex_destruct_string_buf(&buf);
    lclex_destruct_hashmap(&defs);
    lclex_destruct_operator_levels(opdefs);

    return 0;
}
