#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <ctype.h>
#include <string.h>

AST_T *parse_string(parser_t *parser, scope_t *scope) {
  AST_T *ast_string = init_ast(parser->tracker, AST_STRING);
  ast_string->string_value = parser->current_token->value;

  parser_expect(parser, T_STRING);

  ast_string->scope = scope;

  return ast_string;
}

AST_T *parse_char(parser_t *parser, scope_t *scope) {
  AST_T *ast_char = init_ast(parser->tracker, AST_CHAR);
  ast_char->char_value = parser->current_token->value[0];

  parser_expect(parser, T_CHAR);

  ast_char->scope = scope;

  return ast_char;
}

AST_T *parse_int(parser_t *parser, scope_t *scope) {
  AST_T *ast_int = init_ast(parser->tracker, AST_INT);
  ast_int->int_value = atoi(parser->current_token->value);

  parser_expect(parser, T_INT);

  ast_int->scope = scope;

  return ast_int;
}

AST_T *parse_number_range(parser_t *parser, scope_t *scope) {
  AST_T *ast_range = init_ast(parser->tracker, AST_NUMBER_RANGE);

  char *start = strtok(parser->prev_token->value, "..");
  char *end = strtok(NULL, "..");

  if (start[0] == '0' || strtol(start, NULL, 10)) {
    AST_T *ast_int = init_ast(parser->tracker, AST_INT);
    ast_int->int_value = atoi(start);
    ast_range->number_range_start = ast_int;
  } else if (isalpha(start[0])) {
    AST_T *ast_variable = init_ast(parser->tracker, AST_VARIABLE);
    ast_variable->variable_name = start;
    ast_variable->scope = scope;

    ast_range->number_range_start = ast_variable;
  } else {
    log_error(parser->tracker, "number range is not valid");
  }

  if (start[0] == '0' || strtol(end, NULL, 10)) {
    AST_T *ast_int = init_ast(parser->tracker, AST_INT);
    ast_int->int_value = atoi(end);
    ast_range->number_range_end = ast_int;
  } else if (isalpha(end[0])) {
    AST_T *ast_variable = init_ast(parser->tracker, AST_VARIABLE);
    ast_variable->variable_name = end;
    ast_variable->scope = scope;

    ast_range->number_range_end = ast_variable;
  } else {
    log_error(parser->tracker, "number range is not valid");
  }

  ast_range->scope = scope;

  return ast_range;
}

AST_T *parse_float(parser_t *parser, scope_t *scope) {
  AST_T *ast_float = init_ast(parser->tracker, AST_FLOAT);
  ast_float->float_value = atof(parser->current_token->value);

  parser_expect(parser, T_FLOAT);

  ast_float->scope = scope;

  return ast_float;
}

AST_T *parse_hex(parser_t *parser, scope_t *scope) {
  AST_T *ast_hex = init_ast(parser->tracker, AST_INT);
  ast_hex->int_value = (int)strtol(parser->current_token->value, NULL, 16);

  parser_expect(parser, T_INT_HEX);

  ast_hex->scope = scope;

  return ast_hex;
}

AST_T *parse_octal(parser_t *parser, scope_t *scope) {
  AST_T *ast_octal = init_ast(parser->tracker, AST_INT);
  ast_octal->int_value = (int)strtol(parser->current_token->value, NULL, 8);

  parser_expect(parser, T_INT_OCTAL);

  ast_octal->scope = scope;

  return ast_octal;
}

AST_T *parse_binary(parser_t *parser, scope_t *scope) {
  AST_T *ast_binary = init_ast(parser->tracker, AST_INT);
  ast_binary->int_value =
      (int)strtol(parser->current_token->value + 2, NULL, 2);

  parser_expect(parser, T_INT_BINARY);

  ast_binary->scope = scope;

  return ast_binary;
}

AST_T *parse_bool(parser_t *parser, scope_t *scope) {
  AST_T *ast_bool = init_ast(parser->tracker, AST_BOOL);

  if (strcmp(parser->current_token->value, "true") == 0) {
    ast_bool->bool_value = true;
  } else {
    ast_bool->bool_value = false;
  }

  parser_expect(parser, T_BOOL);

  ast_bool->scope = scope;

  return ast_bool;
}
