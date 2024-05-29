#define _GNU_SOURCE

#include "tree.h"
#include "parser.h"
#include <stdio.h>
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

    lclex_parser_signal_t sig = LCLEX_PARSER_SUCCESS;

    while (sig != LCLEX_PARSER_EXIT) {
        printf(">>> ");
        lclex_readline(&buf, stdin);

        char *text = buf.str;
        lclex_node_t *expr;

        sig = lclex_parse_statement(&text, &defs, &expr);

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
    
    lclex_destruct_hashmap(&defs);
    lclex_destruct_string_buf(&buf);

    return 0;
}
