#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../utils/log.h"
#include "../scope/scope.h"

typedef struct PARSER_STRUCT {
  token_t **all_tokens;
  int tokens_size;
  int i;
  token_t *current_token;
  token_t *prev_token;
  token_t *next_token;
  tracker_t *tracker;
} parser_t;

parser_t *init_parser(token_t **all_tokens, int tokens_size);

void print_all_nodes(AST_T *ast);

void parser_expect(parser_t *parser, token_type_t token_type);

AST_T *parse_use_statement(parser_t *parser, scope_t *scope);

AST_T *parse_break_statement(parser_t *parser, scope_t *scope);

AST_T *parse_continue_statement(parser_t *parser, scope_t *scope);

AST_T *parse_conditional_operator(parser_t *parser, scope_t *scope);

AST_T *parse_operator(parser_t *parser, scope_t *scope);

AST_T *parse_loop_loop(parser_t *parser, scope_t *scope);

AST_T *parse_for_loop(parser_t *parser, scope_t *scope);

AST_T *parse_if_statement(parser_t *parser, scope_t *scope);

void add_node_to_tail(AST_T *node, AST_T *parent);

void add_node_to_head(AST_T *node, AST_T *parent);

int parser_expect_check(parser_t *parser, token_type_t token_type);

AST_T *parser_start(parser_t *parser, scope_t *start_scope);

AST_T *parser_hoist(parser_t *parser, AST_T *ast, ast_type_t hoist_type);

AST_T *parser_import(parser_t *parser, AST_T *ast);

AST_T *parse_statement(parser_t *parser, scope_t *scope);

AST_T *parse_block(parser_t *parser, scope_t *parent_scope, scope_t *new_scope,
                   block_type_t block_type);

AST_T *parse_return_statement(parser_t *parser, scope_t *scope);

AST_T *parse_term(parser_t *parser, scope_t *scope);

AST_T *parse_arithmetic_expression(parser_t *parser, scope_t *scope);

AST_T *parse_function_call(parser_t *parser, scope_t *scope);

AST_T *parse_variable_definition(parser_t *parser, scope_t *scope);

AST_T *parse_variable_redefinition(parser_t *parser, scope_t *scope);

AST_T *parse_function_definition(parser_t *parser, scope_t *scope);

AST_T *parse_variable(parser_t *parser, scope_t *scope);

AST_T *parse_function_arg(parser_t *parser, scope_t *scope);

AST_T *parse_string(parser_t *parser, scope_t *scope);

AST_T *parse_char(parser_t *parser, scope_t *scope);

AST_T *parse_int(parser_t *parser, scope_t *scope);

AST_T *parse_hex(parser_t *parser, scope_t *scope);

AST_T *parse_octal(parser_t *parser, scope_t *scope);

AST_T *parse_binary(parser_t *parser, scope_t *scope);

AST_T *parse_float(parser_t *parser, scope_t *scope);

AST_T *parse_bool(parser_t *parser, scope_t *scope);

AST_T *parse_number_range(parser_t *parser, scope_t *scope);

AST_T *parse_id(parser_t *parser, scope_t *scope);
#endif
