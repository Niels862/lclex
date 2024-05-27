#include "tree.h"
#include "parser.h"
#include <stdio.h>

int main() {
    lclex_string_buf_t buf;
    lclex_init_string_buf(&buf);

    printf("> ");
    lclex_readline(&buf, stdin);

    char *text = buf.str;

    lclex_node_t *node = lclex_parse_expression(&text);

    if (node == NULL) {
        fprintf(stderr, "Error occurred while parsing\n");
        return 1;
    }

    lclex_stack_t stack;
    lclex_init_stack(&stack);

    lclex_write_node(node, stdout, &stack);
    printf("\n");
    
    lclex_destruct_stack(&stack);
    lclex_free_node(node);

    return 0;
}
