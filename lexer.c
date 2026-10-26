#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char_lexem_pair_t single_char_tokens[] = { { '=', LX_EQ },
  { '+', LX_PLUS},
  { '-', LX_MINUS},
  { '*', LX_STAR},
  { '/', LX_SLASH},
  { ',', LX_COMMA },
  { ':', LX_COLUMN },
  { ';', LX_SEMICOLUMN },
  { '(', LX_OPAR },
  { ')', LX_CPAR },
  { '[', LX_OSQR_BRA },
  { ']', LX_CSQR_BRA },
  { '{', LX_OCRL_BRA },
  { '}', LX_CCRL_BRA },
  { '<', LX_LT },
  { '>', LX_GT },
  { '^', LX_CARAT },
  { 0, LX_ERR } };

keyword_lexem_pair_t keyword_tokens[] = {
  // Types
  { "char", LX_TYPE_CHAR },
  { "uchar", LX_TYPE_UCHAR },
  { "short", LX_TYPE_SHORT },
  { "ushort", LX_TYPE_USHORT },
  { "int", LX_TYPE_INT },
  { "uint", LX_TYPE_UINT },
  { "long", LX_TYPE_LONG },
  { "ulong", LX_TYPE_ULONG },
  { "llong", LX_TYPE_LLONG },
  { "ullong", LX_TYPE_ULLONG },
  { "float", LX_TYPE_FLOAT },
  { "double", LX_TYPE_DOUBLE },
  { "ldouble", LX_TYPE_LDOUBLE },
  { "bool", LX_TYPE_BOOL },
  { "void", LX_TYPE_VOID },
  // keywords
  { "if", LX_IF },
  { "else", LX_ELSE },
  { "while", LX_WHILE },
  { "defer", LX_DEFER },
  { "ret", LX_RETURN },
  { "return", LX_RETURN },
  { "fn", LX_FUNC },
  { "func", LX_FUNC },
  { NULL, LX_ERR }
};

void lexer_init (FILE* f, lexer_t* l) {
  l->input_file = f;
  l->char_num   = 1;
  l->line_num   = 1;
  l->next_idx   = 0;
  l->byte_num   = 0;  
  l->in_comment = false;
  for (int i = 0; i < LEXER_BUFFER_SIZE; i++) {
    l->next_lexems[i] = lexer_next_token (l);
  }
}

int lexer_peek (lexer_t* l) {
  int c = fgetc (l->input_file);
  ungetc (c, l->input_file);
  return c;
}

int lexer_consume (lexer_t* l) {
  int c = fgetc (l->input_file);
  if (c == '\n') {
    l->line_num++;
    l->char_num = 0;
  }
  l->char_num++;
  l->byte_num++;
  return c;
}

bool lexer_is_operator (int c) {
  return (c == '+') || (c == '-') || (c == '*') || (c == '/');
}

void lexer_consume_spaces (lexer_t* l) {
  int c = lexer_peek (l);
  while (isspace (c) || l->in_comment || c == '#') {
    if (c == '\n' && l->in_comment) {
      l->in_comment = false;
    }
    if (c == '#') {
      l->in_comment = !l->in_comment;
    }
    lexer_consume (l);
    c = lexer_peek (l);
  }
}

lexem_t lexer_lex_integer (lexer_t* l) {
  char buffer[12] = {};
  size_t idx      = 0;
  int c           = lexer_peek (l);

  size_t line_num = l->line_num;
  size_t char_num = l->char_num;
  size_t start_byte = l->byte_num;
  while (isdigit (c)) {
    lexer_consume (l);
    if (idx >= 11) {
      lexer_emit_error ("Integer too long", l->line_num, l->char_num);
      break;
    }
    buffer[idx++] = c;
    c             = lexer_peek (l);
    while (c == '_') {
      lexer_consume (l);
      c = lexer_peek (l);
    }
  }

  lexem_t lex   = (lexem_t){
    .char_num   = char_num,
    .line_num   = line_num,
    .kind       = LX_INT,
    .ival       = atoi (buffer),
    .start_byte = start_byte,
    .end_byte   = l->byte_num
  };
  return lex;
}

lexem_t lexer_lex_kw_identifier (lexer_t* l) {
  char buffer[64] = {};

  size_t idx = 0;
  int c      = lexer_peek (l);

  size_t sbyte    = l->byte_num;
  size_t line_num = l->line_num;
  size_t char_num = l->char_num;

  if (isalpha (c) || c == '_') {
    lexer_consume (l);
    buffer[idx++] = c;
    c             = lexer_peek (l);
  }
  while (isalnum (c) || c == '_') {
    lexer_consume (l);
    if (idx >= 63) {
      lexer_emit_error ("Identifier too long", l->line_num, l->char_num);
      break;
    }
    buffer[idx++] = c;
    c             = lexer_peek (l);
  }

  for (int i = 0; keyword_tokens[i].keyword != NULL; i++) {
    if (strcmp (keyword_tokens[i].keyword, buffer) == 0) {
      lexem_t lex = (lexem_t){
        .kind     = keyword_tokens[i].kind,
        .char_num = char_num,
        .line_num = line_num,
      };
      return lex;
    }
  }

  lexem_t lex = (lexem_t){
    .kind     = LX_IDENT,
    .char_num = char_num,
    .line_num = line_num,
    .start_byte = sbyte,
    .end_byte = l->byte_num
  };
  strcpy (lex.identifier, buffer);
  return lex;
}

lexem_t lexer_next_token (lexer_t* l) {
  lexer_consume_spaces (l);
  int c = lexer_peek (l);

  if (isdigit (c)) {
    return lexer_lex_integer (l);
  }
  
  for (int i = 0; single_char_tokens[i].character != 0; i++) {
    if (c == single_char_tokens[i].character) {
      lexer_consume (l);
      return (lexem_t){ .kind = single_char_tokens[i].kind,
        .line_num             = l->line_num,
        .char_num             = l->char_num };
    }
  }

  if (isalnum (c) || c == '_') {
    return lexer_lex_kw_identifier (l);
  }

  return (lexem_t){ .kind = LX_ERR };
}

void lexer_throw_till_break (lexer_t* l) {
  while (lexer_peek_token (l).kind != LX_SEMICOLUMN &&
         lexer_peek_token (l).kind != LX_CCRL_BRA && lexer_peek_token (l).kind != LX_ERR)
    lexer_consume_token (l);
  lexer_consume_token (l);
}

size_t lexer_get_line (lexer_t* l) { return lexer_peek_token (l).line_num; }

size_t lexer_get_char (lexer_t* l) { return lexer_peek_token (l).char_num; }

void lexer_print_lexem (lexem_t lexem) {
  printf ("<(%zu:%zu) ", lexem.line_num, lexem.char_num);

  for (int i = 0; single_char_tokens[i].character != 0; i++) {
    if (lexem.kind == single_char_tokens[i].kind) {
      printf ("\'%c\' >", single_char_tokens[i].character);
    }
  }
  
  for (int i = 0; keyword_tokens[i].keyword != NULL; i++) {
    if (lexem.kind == keyword_tokens[i].kind) {
      printf ("\'%s\' >", keyword_tokens[i].keyword);
    }
  }

  switch (lexem.kind) {
  case LX_ERR: printf ("ERROR>"); break;
  case LX_INT: printf ("int : %d>", lexem.ival); break;
  case LX_IDENT: printf ("ident : %s>", lexem.identifier); break;
  case LX_TYPE_INT: printf ("type int>"); break;
  default: break;
  }
}

const char* lexer_lexem_str(lexem_kind_t kind)
{ 

  static char temp[2] = {0, 0};
  for (int i = 0; single_char_tokens[i].character != 0; i++) {
    if (kind == single_char_tokens[i].kind) {
      temp[1] = single_char_tokens[i].character;
      return temp;
    }
  }
  
  for (int i = 0; keyword_tokens[i].keyword != NULL; i++) {
    if (kind == keyword_tokens[i].kind) {
      return keyword_tokens[i].keyword;
    }
  }

  switch (kind) {
  case LX_ERR: return "ERROR>"; break;
  case LX_INT: return "LX_INT"; break;
  case LX_IDENT: 
      return "IDENT";
      break;
  default: break;
  }
  return NULL;
}

char lexer_token_to_char(lexem_kind_t kind)
{ 
  for (int i = 0; single_char_tokens[i].character != 0; i++) {
    if (kind == single_char_tokens[i].kind){
      return single_char_tokens[i].character;
    }
  }
  return 0;
}

void lexer_emit_error (const char* err, size_t line_num, size_t char_num) {
  printf ("[LEXER ERROR] (%ld:%ld) %s\n", line_num, char_num, err);
}

lexem_t lexer_peek_token (lexer_t* l) { return l->next_lexems[l->next_idx]; }

lexem_t lexer_peek_at (lexer_t* l, size_t idx) {
  if (idx >= LEXER_BUFFER_SIZE) return (lexem_t){ .kind = LX_ERR };
  return l->next_lexems[(l->next_idx + idx) % LEXER_BUFFER_SIZE];
}

lexem_t lexer_consume_token (lexer_t* l) {
  lexem_t lex                 = l->next_lexems[l->next_idx];
  l->next_lexems[l->next_idx] = lexer_next_token (l);
  l->next_idx                 = (l->next_idx + 1) % LEXER_BUFFER_SIZE;
  return lex;
}
