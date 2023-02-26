#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <ctype.h>
#include <string.h>

token_t *init_token(int type, char *value, lexer_t *lexer);

lexer_t *init_lexer(char *contents) {
  lexer_t *lexer = calloc(1, sizeof(struct LEXER_STRUCT));
  lexer->contents = contents;
  lexer->i = 0;
  lexer->c = contents[lexer->i];
  lexer->prev_c = contents[lexer->i];
  lexer->next_c = contents[lexer->i];
  lexer->tracker = (void *)0;

  return lexer;
}

void advance(lexer_t *lexer) {
  if (lexer->c != '\0' && lexer->i < strlen(lexer->contents)) {
    lexer->i += 1;
    lexer->prev_c = lexer->c;
    lexer->c = lexer->contents[lexer->i];
    lexer->next_c = lexer->contents[lexer->i + 1];

    lexer->tracker->column++;
  }
}

void skip_whitespace(lexer_t *lexer) {
  while (lexer->c == SPACE_CHAR || lexer->c == TAB_CHAR ||
         lexer->c == NEWLINE_CHAR) {

    if (lexer->c == NEWLINE_CHAR) {
      lexer->tracker->line++;
      lexer->tracker->column = 1;
      advance(lexer);
    } else {
      advance(lexer);
    }
  }
}

token_t *get_next_token(lexer_t *lexer) {
  while (
      lexer->c != NULL_CHAR &&
      lexer->i <
          strlen(lexer->contents)) { // this will loop until a token in returned

    // skip items

    if (lexer->c == SPACE_CHAR || lexer->c == TAB_CHAR ||
        lexer->c == NEWLINE_CHAR)
      skip_whitespace(lexer);

    // collect multiple

    if (isalnum(lexer->c) || lexer->c == UNDERSCORE_CHAR)
      return collect_alnum(lexer);

    if (lexer->c == DOUBLE_QUOTE_CHAR)
      return collect_string(lexer);

    if (lexer->c == SINGLE_QUOTE_CHAR) {
      return collect_char(lexer);
    }

    if (lexer->c == FSLASH_CHAR) {
      token_t *slash_token = collect_fslash(lexer);
      if (slash_token->type != T_COMMENT) {
        return slash_token;
      }

      continue;
    }

    if (lexer->c == LANGLE_CHAR) {
      if (lexer->next_c == EQUAL_CHAR) {
        advance(lexer);
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR, "<=", lexer));
      } else {
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR,
                              get_current_char_as_string(lexer), lexer));
      }
    }

    if (lexer->c == RANGLE_CHAR) {
      if (lexer->next_c == EQUAL_CHAR) {
        advance(lexer);
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR, ">=", lexer));
      } else if (lexer->prev_c == MINUS_CHAR) {
        return advance_with_token(
            lexer,
            init_token(T_RANGLE, get_current_char_as_string(lexer), lexer));
      } else {
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR,
                              get_current_char_as_string(lexer), lexer));
      }
    }

    if (lexer->c == EXCLAMATION_CHAR) {
      if (lexer->next_c == EQUAL_CHAR) {
        advance(lexer);
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR, "!=", lexer));
      } else {
        return advance_with_token(
            lexer, init_token(T_LOGICAL_OPERATOR,
                              get_current_char_as_string(lexer), lexer));
      }
    }

    if (lexer->c == EQUAL_CHAR) {
      if (lexer->next_c == EQUAL_CHAR) {
        advance(lexer);
        return advance_with_token(
            lexer, init_token(T_CONDITIONAL_OPERATOR, "==", lexer));
      }
    }

    if (lexer->c == AMPERSAND_CHAR) {
      if (lexer->next_c == AMPERSAND_CHAR) {
        advance(lexer);
        return advance_with_token(lexer,
                                  init_token(T_LOGICAL_OPERATOR, "&&", lexer));
      }
    }

    if (lexer->c == VBAR_CHAR) { // the char is |
      if (lexer->next_c == VBAR_CHAR) {
        advance(lexer);
        return advance_with_token(lexer,
                                  init_token(T_LOGICAL_OPERATOR, "||", lexer));
      }
    }

    // collect single

    switch (lexer->c) {

    case EQUAL_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_EQUALS, get_current_char_as_string(lexer), lexer));
      break;

    case PLUS_CHAR:
      return advance_with_token(
          lexer, init_token(T_ARITHMETIC_OPERATOR,
                            get_current_char_as_string(lexer), lexer));
      break;

    case MINUS_CHAR:
      return advance_with_token(
          lexer, init_token(T_ARITHMETIC_OPERATOR,
                            get_current_char_as_string(lexer), lexer));
      break;

    case ASTERIX_CHAR:
      return advance_with_token(
          lexer, init_token(T_ARITHMETIC_OPERATOR,
                            get_current_char_as_string(lexer), lexer));
      break;

    case SEMI_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_SEMICOLON, get_current_char_as_string(lexer), lexer));
      break;

    case COLON_CHAR:
      return advance_with_token(
          lexer, init_token(T_COLON, get_current_char_as_string(lexer), lexer));
      break;

    case DOT_CHAR:
      return advance_with_token(
          lexer, init_token(T_DOT, get_current_char_as_string(lexer), lexer));
      break;

    case LPAREN_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_LPAREN, get_current_char_as_string(lexer), lexer));
      break;

    case RPAREN_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_RPAREN, get_current_char_as_string(lexer), lexer));
      break;

    case LBRACE_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_LBRACE, get_current_char_as_string(lexer), lexer));
      break;

    case RBRACE_CHAR:
      return advance_with_token(
          lexer,
          init_token(T_RBRACE, get_current_char_as_string(lexer), lexer));
      break;

    case COMMA_CHAR:
      return advance_with_token(
          lexer, init_token(T_COMMA, get_current_char_as_string(lexer), lexer));
      break;

    case TILDE_CHAR:
      return advance_with_token(
          lexer, init_token(T_TILDE, get_current_char_as_string(lexer), lexer));
      break;

    // if we encountered a char that was not handled
    default: {
      if (lexer->c == NULL_CHAR) {
        return init_token(T_EOF, "\0", lexer);
      }

      log_error(lexer->tracker, "unexpected character `%c` -%d-", lexer->c,
                lexer->c);
    }
    }
  }

  // add an EOF token after everything
  return init_token(T_EOF, "\0", lexer);
}

token_t *collect_string(lexer_t *lexer) {
  advance(lexer);

  char *value = calloc(1, sizeof(char));
  value[0] = NULL_CHAR;

  while (lexer->c != DOUBLE_QUOTE_CHAR) {
    char *s = get_current_char_as_string(lexer);
    value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
    strcat(value, s);

    advance(lexer);
  }

  advance(lexer);

  return init_token(T_STRING, value, lexer);
}

token_t *collect_fslash(lexer_t *lexer) {
  char *value = calloc(1, sizeof(char));
  value[0] = NULL_CHAR;

  char first_char = lexer->c;

  advance(lexer);
  char second_char = lexer->c;

  if (first_char == FSLASH_CHAR &&
      second_char == FSLASH_CHAR) { // single line comment
    advance(lexer);
    while (lexer->c != NEWLINE_CHAR && lexer->i < strlen(lexer->contents)) {
      char *s = get_current_char_as_string(lexer);
      value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
      strcat(value, s);

      advance(lexer);
    }
  } else if (first_char == FSLASH_CHAR &&
             second_char == ASTERIX_CHAR) { // multiline comment
    advance(lexer);
    while (lexer->c != ASTERIX_CHAR && lexer->contents[lexer->i + 1] &&
           lexer->i < strlen(lexer->contents)) {

      if (lexer->c == NEWLINE_CHAR) {
        lexer->tracker->line++;
      }

      char *s = get_current_char_as_string(lexer);
      value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
      strcat(value, s);

      advance(lexer);
    }
    advance(lexer); // asterix
    advance(lexer); // FSLASH
  } else {
    char *str = calloc(2, sizeof(char));
    str[0] = first_char;
    str[1] = NULL_CHAR;

    return advance_with_token(lexer,
                              init_token(T_ARITHMETIC_OPERATOR, str, lexer));
  }

  return init_token(T_COMMENT, value, lexer);
}

token_t *collect_char(lexer_t *lexer) {
  advance(lexer);

  char *str = calloc(2, sizeof(char));
  str[0] = lexer->c;
  str[1] = NULL_CHAR;

  advance(lexer);
  advance(lexer);

  return init_token(T_CHAR, str, lexer);
}

token_t *advance_with_token(lexer_t *lexer, token_t *token) {
  advance(lexer);

  return token;
}

token_t *collect_alnum(lexer_t *lexer) {
  char *value = calloc(1, sizeof(char));
  value[0] = NULL_CHAR;

  while (isalnum(lexer->c) || lexer->c == UNDERSCORE_CHAR ||
         lexer->c == DOT_CHAR) {
    char *s = get_current_char_as_string(lexer);
    value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
    strcat(value, s);

    advance(lexer);
  }

  // Number range
  // NOTE: does not support arithmetic expressions
  if (strstr(value, "..") != 0) {
    int dots = 0;

    for (int i = 0; value[i] != '\0'; i++) {

      if (!isdigit(value[i]) && !isalpha(value[i]) && value[i] != '.') {
        log_error(lexer->tracker, "number range is invalid");
      }

      if (value[i] == '.') {
        dots++;
        if (dots > 2) {
          log_error(lexer->tracker, "number range is invalid");
        }

        if (dots == 2 && !isdigit(value[i + 1]) && !isalpha(value[i + 1])) {
          log_error(lexer->tracker, "number range is invalid");
        }
      }
    }

    return init_token(T_NUMBER_RANGE, value, lexer);
  }

  if (isdigit(value[0])) { // does it start with a number?
    NumberFormat format = check_number(value);

    switch (format) {
    case INVALID: {
      log_error(lexer->tracker, "invalid number literal `%s`", value);
    }

    case DECIMAL: {
      return init_token(T_INT, value, lexer);
    }

    case OCTAL: {
      return init_token(T_INT_OCTAL, value, lexer);
    }

    case HEXADECIMAL: {
      return init_token(T_INT_HEX, value, lexer);
    }

    case BINARY: {
      return init_token(T_INT_BINARY, value, lexer);
    }

    case FLOAT: {
      return init_token(T_FLOAT, value, lexer);
    }
    }

  } else if (strcmp(value, "true") == 0) {
    return init_token(T_BOOL, value, lexer);
  } else if (strcmp(value, "false") == 0) {
    return init_token(T_BOOL, value, lexer);
  } else {
    return init_token(T_ID, value, lexer);
  }
}

char *get_current_char_as_string(lexer_t *lexer) {
  char *str = calloc(2, sizeof(char));
  str[0] = lexer->c;
  str[1] = NULL_CHAR;

  return str;
}
