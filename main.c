#include "arena.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>

int compile_per_file(FILE *file) {
  lexer_t lexer;
  arena_t *a = arena_new(10000, 0);
  parser_ctx ctx = {
    .a = a,
    .l = &lexer
  };
  lexer_init(file, &lexer);

  parse_result res = parse_statement(ctx);
  if (res.is_ok) {
    print_ast(res.result);
    printf("\n");
  } else {
    printf("[PARSER ERROR](%zu:%zu) %s\n", res.line_num, res.char_num, res.err);
  }

  printf("arena : space taken : %zu\n", arena_get_taken(a));

  size_t err_len;
  error_t* errors = arena_get_errors(a, &err_len);
  for(size_t i = 0; i < err_len; i++)
  {
    printf("[ERROR (%zu:%zu)] %s\n", errors[i].line_num, errors[i].char_num, errors[i].msg);
  }
  printf("Got %zu errors\n", err_len);

  arena_free(a);

  return 0;
}

int main(int argc, char **argv) {
  printf("Hello from %s\n", argv[0]);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      compile_per_file(f);
      printf("\n");
      fclose(f);
    }
  } else {
    compile_per_file(stdin);
  }
}
