#include "tree.h"
#include "parser.h"
#include <stdio.h>

int main() {
    lclex_string_buf_t buf;
    lclex_init_string_buf(&buf);

    while (true) {
        printf("> ");
        lclex_readline(&buf, stdin);

        char *text = buf.str;
        lclex_node_t *expr = lclex_parse_expression(&text);

        if (expr == NULL) {
            continue;
        }

        lclex_reduce_expression(&expr, 1000);

        lclex_write_node(expr, stdout);

        lclex_free_node(expr);
    }
    
    lclex_destruct_string_buf(&buf);

    return 0;
}
