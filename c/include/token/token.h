#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  T_ID,
  T_EQUALS,
  T_STRING,
  T_CHAR,
  T_COLON,
  T_SEMICOLON,
  T_INT_HEX,
  T_INT_OCTAL,
  T_INT_BINARY,
  T_INT,
  T_FLOAT,
  T_BOOL,
  T_NUMBER_RANGE,
  T_LPAREN,
  T_RPAREN,
  T_RBRACE,
  T_LBRACE,
  T_RANGLE,
  T_LANGLE,
  T_COMMA,
  T_TILDE,
  T_DOT,
  T_ARITHMETIC_OPERATOR,
  T_CONDITIONAL_OPERATOR,
  T_LOGICAL_OPERATOR,
  T_COMMENT,
  T_EOF
} token_type_t;

typedef struct TOKEN_STRUCT {
  token_type_t type;
  char *value;
  int line;
  int column;
} token_t;

typedef struct TOKEN_BUNDLE{
 token_t** all_tokens;
 int tokens_size;
} bundle_t;


#endif
