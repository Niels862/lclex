#include "tree.h"
#include "parser.h"
#include <stdio.h>

int main() {
    lclex_string_buf_t buf;
    lclex_init_string_buf(&buf);

    lclex_hashmap_t defs;
    lclex_init_string_hashmap(&defs, lclex_free_node);

    lclex_parser_signal_t sig = LCLEX_PARSER_SUCCESS;

    while (sig != LCLEX_PARSER_EXIT) {
        printf("> ");
        lclex_readline(&buf, stdin);

        char *text = buf.str;
        lclex_node_t *expr;

        sig = lclex_parse_statement(&text, &defs, &expr);

        if (expr == NULL) {
            continue;
        }

        lclex_reduce_expression(&expr, 1000);

        lclex_write_node(expr, stdout);

        lclex_free_node(expr);
    }
    
    lclex_destruct_hashmap(&defs);
    lclex_destruct_string_buf(&buf);

    return 0;
}
