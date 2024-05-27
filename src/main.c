#include "tree.h"
#include "parser.h"
#include <stdio.h>

int main() {
    char expr[] = "\\x.\\y.\\z.(x y z)";
    char *text = expr;

    lclex_node_t *node = lclex_parse_expression(&text);

    if (node == NULL) {
        fprintf(stderr, "Error occurred while parsing\n");
        return 1;
    }

    lclex_string_stack_t stack;
    lclex_init_string_stack(&stack);

    lclex_write_node(node, stdout, &stack);
    printf("\n");
    
    lclex_destruct_string_stack(&stack);
    lclex_free_node(node);

    return 0;
}
