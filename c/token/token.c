#include "../include/lexer/lexer.h"
#include <stdlib.h>


token_t *init_token(int type, char *value, lexer_t *lexer) {
  token_t *token = calloc(1, sizeof(struct TOKEN_STRUCT));
  token->type = type;
  token->value = value;
  token->line = lexer->tracker->line;
  token->column = lexer->tracker->column;

  return token;
}
