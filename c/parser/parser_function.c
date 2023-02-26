#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <string.h>

// block of code enclosed in braces which has it's private scope
AST_T *parse_block(parser_t *parser, scope_t *parent_scope, scope_t *new_scope,
                   block_type_t block_type) {
  parser_expect(parser, T_LBRACE);

  AST_T *block = init_ast(parser->tracker, AST_BLOCK);
  block->scope = new_scope;

  block->scope->tracker = parser->tracker;
  block->scope->parent_scope = parent_scope;
  block->block_type = block_type;

  int block_has_returned = 0;

  for (;;) {

    if (block->block_size < 1) {
      block->block_values = calloc(1, sizeof(struct AST_STRUCT *));
    }

    if (parser->current_token->type == T_RBRACE) {
      // implicitly add an empty return if none was encountered
      if (block_type == BLOCK_FN && !block_has_returned) {
        // if we specified a return type
        if (block->scope->return_type != AST_NULL) {
          log_error(parser->tracker,
                    "return type was specified but no value was returned");
        }

        AST_T *return_statement =
            init_ast(parser->tracker, AST_RETURN_STATEMENT);

        block->scope->return_type = AST_NULL;

        block->block_size += 1;
        block->block_values =
            realloc(block->block_values,
                    block->block_size * sizeof(struct AST_STRUCT *));
        block->block_values[block->block_size - 1] = return_statement;
      }

      break;
    } else {
      AST_T *ast_statement = parse_statement(parser, block->scope);

      if (ast_statement->type == AST_RETURN_STATEMENT) {
        block_has_returned = 1;
      }

      if (ast_statement) {
        ast_statement->parent_block = block;

        if (ast_statement->type == AST_FUNCTION_DEFINITION) {
          ast_statement->function_definition_body->parent_block = block;
        } else if (ast_statement->type == AST_IF_STATEMENT) {
          ast_statement->if_statement_body->parent_block = block;
        } else if (ast_statement->type == AST_FOR_LOOP) {
          ast_statement->for_loop_body->parent_block = block;
        } else if (ast_statement->type == AST_LOOP_LOOP) {
          ast_statement->loop_loop_body->parent_block = block;
        }

        block->block_size += 1;
        block->block_values =
            realloc(block->block_values,
                    block->block_size * sizeof(struct AST_STRUCT *));
        block->block_values[block->block_size - 1] = ast_statement;
      } else {
        log_error(parser->tracker, "no statement was returned from "
                                   "parser_parse_statement");
      }

      if (parser->prev_token->type != T_RBRACE) {
        parser_expect(parser, T_SEMICOLON);
      }
    }
  }

  parser_expect(parser, T_RBRACE);

  return block;
}

// a statment is a line of code which ends with a semicolon
AST_T *parse_statement(parser_t *parser, scope_t *scope) {
  switch (parser->current_token->type) {
  case T_ID:
    return parse_id(parser, scope);

  case T_EOF:
    return init_ast(parser->tracker, AST_EOF);
  }

  log_error(parser->tracker,
            "current token while parsing statement is not an "
            "identifier. Type: "
            "%s, value `%s` ",
            token_type_to_string(parser->current_token->type),
            parser->current_token->value);
}

AST_T *parse_return_statement(parser_t *parser, scope_t *scope) {
  parser_expect(parser, T_ID); // expect return keyword

  AST_T *return_statement = init_ast(parser->tracker, AST_RETURN_STATEMENT);

  return_statement->scope = scope;

  bool type_specified = false;
  if (scope->return_type != AST_NULL) {
    type_specified = true;
  }

  bool got_value = false;
  if (parser->current_token->type != T_SEMICOLON) {
    got_value = true;
  }

  // if we specified a return type but no value was returned
  if (type_specified && !got_value) {
    log_error(parser->tracker,
              "return type was specified but no value was returned");
  }

  // if a value was returned, save it
  if (got_value) {
    AST_T *return_value = parse_term(parser, scope);

    return_statement->returned_value = return_value;
  }

  // if a value was returned, but function signature did not specify a value,
  // infer the type
  if (got_value && scope->return_type == AST_NULL) {

    scope->return_type = return_statement->returned_value->type;
  }

  // if function signature specified a type, but the returned value has a
  // different type
  if (type_specified &&
      return_statement->returned_value->type != AST_VARIABLE &&
      return_statement->returned_value->type != scope->return_type) {
    log_error(parser->tracker,
              "return type of function does not match returned "
              "value. Expected "
              "%s found %s",
              ast_type_to_str(parser->tracker, scope->return_type),
              ast_type_to_str(parser->tracker,
                              return_statement->returned_value->type));
  }

  return return_statement;
}

AST_T *parse_function_call(parser_t *parser, scope_t *scope) {
  AST_T *function_call = init_ast(parser->tracker, AST_FUNCTION_CALL);

  function_call->scope = scope;

  function_call->function_call_name = parser->prev_token->value;

  parser_expect(parser, T_LPAREN);

  function_call->function_call_arguments_size =
      0; // initialize the arguments count to zero

  int passed = parser_expect_check(
      parser, T_RPAREN); // check if the next char is right parentheses

  if (!passed) { // if we actually have arguments
    while (parser->current_token->type != T_RPAREN) {

      AST_T *ast_expr = parse_term(parser, scope);
      function_call->function_call_arguments_size += 1;
      function_call->function_call_arguments =
          realloc(function_call->function_call_arguments,
                  function_call->function_call_arguments_size *
                      sizeof(struct AST_STRUCT *));
      function_call->function_call_arguments
          [function_call->function_call_arguments_size - 1] = ast_expr;

      // check if there are additional arguments
      int more = parser_expect_check(parser, T_COMMA);
      if (more) {                       // there are additional arguments
        parser_expect(parser, T_COMMA); // expect a comma
      }
    }
  }

  parser_expect(parser, T_RPAREN);

  return function_call;
}

AST_T *parse_function_definition(parser_t *parser, scope_t *scope) {
  AST_T *ast = init_ast(parser->tracker, AST_FUNCTION_DEFINITION);

  ast->scope = scope;

  parser_expect(parser, T_ID); // expect the fn identifier

  char *function_name = parser->current_token->value;
  ast->function_definition_name =
      calloc((strlen(function_name) + 1), sizeof(char));
  strcpy(ast->function_definition_name, function_name);

  parser_expect(parser, T_ID); // expect the function name

  parser_expect(parser, T_LPAREN); // expect the left parenthesis

  ast->function_definition_args_size =
      0; // initialize the arguments count to zero

  int passed = parser_expect_check(
      parser, T_RPAREN); // check if the next char is right parentheses

  if (!passed) { // if we actually have arguments
    if (parser_expect_check(parser, T_TILDE)) {
      ast->function_definition_is_variadic = 1;
      parser_expect(parser, T_TILDE);
    } else {

      while (parser->current_token->type != T_RPAREN) {
        ast->function_definition_args_size += 1; // increment the arguments size

        if (ast->function_definition_args_size == 1) {
          ast->function_definition_args = calloc(
              1,
              sizeof(struct AST_STRUCT *)); // allocate memory for the arguments
        }

        // reallocate memory to accomodate the new variable
        ast->function_definition_args = realloc(
            ast->function_definition_args,
            ast->function_definition_args_size * sizeof(struct AST_STRUCT *));

        AST_T *arg = parse_function_arg(parser, scope);

        // check if the type was specified
        int specified = parser_expect_check(parser, T_COLON);
        if (specified) {
          parser_expect(parser, T_COLON);

          // save the data type that was specified
          char *data_type = parser->current_token->value;

          parser_expect(parser, T_ID);

          ast_type_t formal_type = str_to_ast_type(parser, data_type);
          arg->type = formal_type;
        }

        ast->function_definition_args[ast->function_definition_args_size - 1] =
            arg;

        // check if there are additional arguments
        int more = parser_expect_check(parser, T_COMMA);
        if (more) {                       // there are additional arguments
          parser_expect(parser, T_COMMA); // expect a comma
        }
      }
    }
  }

  parser_expect(parser, T_RPAREN);

  ast_type_t return_type;

  int pass = parser_expect_check(parser, T_ARITHMETIC_OPERATOR);
  if (pass) {
    char *current_value = parser->current_token->value;
    parser_expect(parser, T_ARITHMETIC_OPERATOR);

    if (strcmp(current_value, "-") != 0) {
      log_error(parser->tracker, "unexpected arithmetic operator `%s`",
                current_value);
    }

    parser_expect(parser, T_RANGLE);

    // save the data type that was specified
    char *data_type = parser->current_token->value;

    parser_expect(parser, T_ID);

    ast_type_t formal_type = str_to_ast_type(parser, data_type);
    return_type = formal_type;
  } else {
    return_type = AST_NULL;
  }

  scope_t *new_scope = init_scope();
  new_scope->return_type = return_type;

  ast->function_definition_body =
      parse_block(parser, scope, new_scope, BLOCK_FN);

  return ast;
}

AST_T *parse_function_arg(parser_t *parser, scope_t *scope) {
  char *token_value = parser->current_token->value;

  AST_T *ast_variable = init_ast(parser->tracker, AST_VARIABLE);
  ast_variable->variable_name = token_value;

  ast_variable->scope = scope;

  parser_expect(parser, T_ID);

  return ast_variable;
}
