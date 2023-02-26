#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <string.h>

void print_all_tokens(token_t **all_tokens) {
  int i = 0;

  for (;;) {
    token_t *token = all_tokens[i];

    log_print("[%s => %s]", token->value, token_type_to_string(token->type));

    if (token->type == T_EOF) {
      break;
    }

    i++;
  }

  log_print("Found %d tokens", i);
}

bundle_t get_all_tokens(lexer_t *lexer) {
  int tokens_count = 0;

  token_t **all_tokens = malloc(sizeof(token_t *));

  for (;;) {
    token_t *token = get_next_token(lexer);

    all_tokens[tokens_count] = token;
    all_tokens = realloc(all_tokens, sizeof(token_t *) * (tokens_count + 2));
    tokens_count++;

    if (token->type == T_EOF) {
      break;
    }
  }

  lexer->i = 0;
  lexer->c = lexer->contents[lexer->i];

  bundle_t bundle = {all_tokens, tokens_count};
  return bundle;
}

char *token_type_to_string(token_type_t type) {
  char *token_type_string[] = {"ID",
                               "EQUALS",
                               "STRING",
                               "CHAR",
                               "COLON",
                               "SEMICOLON",
                               "INT_HEX",
                               "INT_OCTAL",
                               "INT_BINARY",
                               "INT",
                               "FLOAT",
                               "BOOL",
                               "NUMBER_RANGE",
                               "LPAREN",
                               "RPAREN",
                               "RBRACE",
                               "LBRACE",
                               "RANGLE",
                               "LANGLE",
                               "COMMA",
                               "TILDE",
                               "DOT",
                               "ARITHMETIC_OPERATOR",
                               "CONDITIONAL_OPERATOR",
                               "LOGICAL_OPERATOR",
                               "COMMENT",
                               "EOF"};

  return token_type_string[type];
}

token_type_t str_to_token_type(parser_t *parser, char *data_type) {
  if (strcmp(data_type, "bool") == 0) {
    return T_BOOL;
  } else if (strcmp(data_type, "int") == 0) {
    return T_INT;
  } else if (strcmp(data_type, "float") == 0) {
    return T_FLOAT;
  } else if (strcmp(data_type, "char") == 0) {
    return T_CHAR;
  } else if (strcmp(data_type, "str") == 0) {
    return T_STRING;
  } else {
    log_error(parser->tracker, "%s is not a type", data_type);
  }
}

char *token_type_to_str(parser_t *parser, token_type_t type) {
  switch (type) {
  case T_BOOL:
    return "BOOL";
  case T_INT:
    return "INT";
  case T_FLOAT:
    return "FLOAT";
  case T_CHAR:
    return "CHAR";
  case T_STRING:
    return "STR";
  default: {
    log_error(parser->tracker, "%d is not a type at line %d", type);
  }
  }
}
