#ifndef LEXER_H
#define LEXER_H
#include "../utils/log.h"
#include "../token/token.h"

typedef struct LEXER_STRUCT {
  char c;
  unsigned int i;
  char prev_c;
  char next_c;
  char *contents;
  tracker_t *tracker;
} lexer_t;

lexer_t *init_lexer(char *contents);

void advance(lexer_t *lexer);

void skip_whitespace(lexer_t *lexer);

token_t *get_next_token(lexer_t *lexer);

token_t *collect_string(lexer_t *lexer);

token_t *collect_fslash(lexer_t *lexer);

token_t *collect_char(lexer_t *lexer);

token_t *collect_alnum(lexer_t *lexer);

token_t *advance_with_token(lexer_t *lexer, token_t *token);

char *get_current_char_as_string(lexer_t *lexer);
#endif
