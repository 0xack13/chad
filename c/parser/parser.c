// #include "../include/utils/log.h"
#include "../include/utils/utils.h"
#include "../include/libchad/libchad.h"

#include <string.h>

AST_T *init_ast(tracker_t *tracker, int type);

parser_t *init_parser(token_t **all_tokens, int tokens_size) {
  parser_t *parser = calloc(1, sizeof(struct PARSER_STRUCT));
  parser->all_tokens = all_tokens;
  parser->tokens_size = tokens_size;
  parser->i = 0;
  parser->current_token = all_tokens[parser->i];
  parser->prev_token = all_tokens[parser->i];
  parser->next_token = all_tokens[parser->i];
  parser->tracker = (void *)0;

  return parser;
}

// expect the token to be a certain type and panic if not
// used to enforce the rules of the grammer
// also advances the parser to the next token
void parser_expect(parser_t *parser, token_type_t token_type) {
  if (parser->current_token->type == token_type) {
    parser->prev_token = parser->all_tokens[parser->i];
    parser->current_token = parser->all_tokens[parser->i + 1];
    parser->next_token = parser->all_tokens[parser->i + 2];
    parser->i++;

    parser->tracker->line = parser->prev_token->line;
    parser->tracker->column = parser->prev_token->column;
  } else {

    // get it? unexpected token lol :)
    log_error(parser->tracker,
              "unexpected token. Expected %s but found %s with "
              "value `%s` ",
              token_type_to_string(token_type),
              token_type_to_string(parser->current_token->type),
              parser->current_token->value);
  }
}

// check if the token is a certain type
// used for optional grammer
// returns a boolean value
int parser_expect_check(parser_t *parser, token_type_t token_type) {
  if (parser->current_token->type == token_type) {
    return 1;
  } else {
    return 0;
  }
}

AST_T *parser_start(parser_t *parser, scope_t *start_scope) {
  AST_T *block = init_ast(parser->tracker, AST_BLOCK);
  block->scope = start_scope;
  block->parent_block = NULL;
  block->scope->parent_scope = NULL;
  block->main_block = 1;

  for (;;) {

    if (block->block_size < 1) {
      block->block_values = calloc(1, sizeof(struct AST_STRUCT *));
    }

    if (parser->current_token->type == T_EOF) {

      break;
    } else {
      AST_T *ast_statement = parse_statement(parser, block->scope);

      if (!ast_statement) {
        log_error(parser->tracker, "no statement was returned from "
                                   "parse_statement");
      }

      if (ast_statement->type == AST_FUNCTION_CALL) {
        log_warning(parser->tracker,
                    "Function call %s() will not be "
                    "reached",
                    ast_statement->function_call_name);
      } else {
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
      }

      if (parser->prev_token->type != T_RBRACE) {
        parser_expect(parser, T_SEMICOLON);
      }
    }
  }

  return block;
}

AST_T *parser_hoist(parser_t *parser, AST_T *ast, ast_type_t hoist_type) {
  int function_count = 0;

  int i;
  for (i = 0; i < ast->block_size; i++) {
    if (ast->block_values[i]->type == hoist_type) {
      function_count++;
    }
  }

  int non_function_count = ast->block_size - function_count;
  AST_T **new_compound_value =
      (AST_T **)malloc(ast->block_size * sizeof(AST_T *));

  int new_i = 0;
  for (i = 0; i < ast->block_size; i++) {
    if (ast->block_values[i]->type == hoist_type) {
      new_compound_value[new_i++] = ast->block_values[i];
    }
  }

  for (i = 0; i < ast->block_size; i++) {
    if (ast->block_values[i]->type != hoist_type) {
      new_compound_value[new_i++] = ast->block_values[i];
    }
  }

  free(ast->block_values);
  ast->block_values = new_compound_value;

  return ast;
}

AST_T *parser_import(parser_t *parser, AST_T *main_ast) {
  int i;

  for (int i = 0; i < main_ast->block_size; i++) {
    AST_T *current_node = main_ast->block_values[i];

    if (current_node->type == AST_USE_STATEMENT) {
      char *file_source;

      if (strcmp(current_node->use_file_path, "core") == 0) {
        file_source = get_core_source();
      } else {
        char *path = strcat(current_node->use_file_path, ".chad");

        file_source = read_file_to_string(path);

        if (strlen(file_source) < 2) {
          log_warning(parser->tracker, "IMPORT - %s is empty", path);
        }
      }

      // LEXER
      tracker_t *lexer_tracker = init_tracker();
      lexer_tracker->filename = current_node->use_file_path;
      lexer_tracker->mode = LEXER;

      lexer_t *import_lexer = init_lexer(file_source);
      import_lexer->tracker = lexer_tracker;
      bundle_t import_bundle = get_all_tokens(import_lexer);
      token_t **import_all_tokens = import_bundle.all_tokens;

      // PARSER
      tracker_t *parser_tracker = init_tracker();
      parser_tracker->filename = current_node->use_file_path;
      parser_tracker->mode = PARSER;

      parser_t *import_parser =
          init_parser(import_all_tokens, import_bundle.tokens_size);

      import_parser->tracker = parser_tracker;
      import_parser->tracker->line = 1;
      import_parser->tracker->column = 1;

      AST_T *ast_root = parser_start(import_parser, main_ast->scope);

      AST_T *import_root =
          parser_hoist(import_parser, ast_root, AST_USE_STATEMENT);
      import_root = parser_import(import_parser, import_root);
      import_root =
          parser_hoist(import_parser, import_root, AST_FUNCTION_DEFINITION);

      int found = 0;
      for (int j = 0; j < import_root->block_size; j++) {
        if (import_root->block_values[j]->type == AST_FUNCTION_DEFINITION &&
            (strcmp(current_node->use_function_name, "*") == 0 ||
             strcmp(current_node->use_function_name,
                    import_root->block_values[j]->function_definition_name) ==
                 0)) {

          add_node_to_tail(import_root->block_values[j], main_ast);

          found = 1;
        }
        if (j == import_root->block_size - 1 && !found) {
          log_error(parser->tracker, "function `%s` not found in %s",
                    current_node->use_function_name,
                    current_node->use_file_path);
        }
      }
    }
  }

  return main_ast;
}

void print_all_nodes(AST_T *ast) {
  for (int i = 0; i < ast->block_size; i++) {
    AST_T *current_node = ast->block_values[i];

    if (current_node->type == AST_FUNCTION_DEFINITION) {
      log_print("function def %s", current_node->function_definition_name);
    } else if (current_node->type == AST_USE_STATEMENT) {
      log_print("use %s from %s", current_node->use_function_name,
                current_node->use_file_path);
    } else if (current_node->type == AST_FUNCTION_CALL) {
      log_print("function call %s", current_node->function_call_name);
    } else if (current_node->type == AST_EOF) {
      log_print("EOF");
    } else {
      log_print("node %d", current_node->type);
    }
  }
}

AST_T *parse_term(parser_t *parser, scope_t *scope) {
  if (parser->prev_token->type != T_ARITHMETIC_OPERATOR &&
      parser->next_token->type == T_ARITHMETIC_OPERATOR) {
    return parse_arithmetic_expression(parser, scope);
  }

  switch (parser->current_token->type) {
  case T_STRING:
    return parse_string(parser, scope);
  case T_CHAR:
    return parse_char(parser, scope);
  case T_INT:
    return parse_int(parser, scope);
  case T_NUMBER_RANGE:
    return parse_number_range(parser, scope);
  case T_INT_HEX:
    return parse_hex(parser, scope);
  case T_INT_OCTAL:
    return parse_octal(parser, scope);
  case T_INT_BINARY:
    return parse_binary(parser, scope);
  case T_FLOAT:
    return parse_float(parser, scope);
  case T_BOOL:
    return parse_bool(parser, scope);

  case T_ID:
    return parse_id(parser, scope);
  }

  // this code is only reached when i expect to parse a term, but the
  // expression is not a valid type or identifier
  log_error(parser->tracker,
            "term is neither a type nor an identifier. Type: "
            "%s, value `%s` ",
            token_type_to_string(parser->current_token->type),
            parser->current_token->value);
}

AST_T *parse_operator(parser_t *parser, scope_t *scope) {
  AST_T *operator= init_ast(parser->tracker, AST_ARITHMETIC_OPERATOR);

  if (strcmp(parser->current_token->value, "+") == 0) {
    operator->operator_value = O_ADD;
  } else if (strcmp(parser->current_token->value, "-") == 0) {
    operator->operator_value = O_SUB;
  } else if (strcmp(parser->current_token->value, "*") == 0) {
    operator->operator_value = O_MUL;
  } else if (strcmp(parser->current_token->value, "/") == 0) {
    operator->operator_value = O_DIV;
  } else {
    log_error(parser->tracker, "arithmetic operator type is invalid");
  }

  parser_expect(parser, T_ARITHMETIC_OPERATOR);

  return operator;
}

AST_T *parse_logical_operator(parser_t *parser, scope_t *scope) {
  AST_T *operator= init_ast(parser->tracker, AST_LOGICAL_OPERATOR);

  if (strcmp(parser->current_token->value, "!") == 0) {
    operator->logical_operator_value = LO_NOT;
  } else if (strcmp(parser->current_token->value, "&&") == 0) {
    operator->logical_operator_value = LO_AND;
  } else if (strcmp(parser->current_token->value, "||") == 0) {
    operator->logical_operator_value = LO_OR;
  } else {
    log_error(parser->tracker, "logical operator type is invalid");
  }

  parser_expect(parser, T_LOGICAL_OPERATOR);

  return operator;
}

AST_T *parse_conditional_operator(parser_t *parser, scope_t *scope) {
  AST_T *operator= init_ast(parser->tracker, AST_CONDITIONAL_OPERATOR);

  if (strcmp(parser->current_token->value, ">") == 0) {
    operator->conditional_operator_value = CON_GT;
  } else if (strcmp(parser->current_token->value, "<") == 0) {
    operator->conditional_operator_value = CON_LT;
  } else if (strcmp(parser->current_token->value, "==") == 0) {
    operator->conditional_operator_value = CON_EQ;
  } else if (strcmp(parser->current_token->value, "!=") == 0) {
    operator->conditional_operator_value = CON_NEQ;
  } else if (strcmp(parser->current_token->value, ">=") == 0) {
    operator->conditional_operator_value = CON_GTE;
  } else if (strcmp(parser->current_token->value, "<=") == 0) {
    operator->conditional_operator_value = CON_LTE;
  } else {
    log_error(parser->tracker, "conditional operator type is invalid");
  }

  parser_expect(parser, T_CONDITIONAL_OPERATOR);

  return operator;
}

AST_T *parse_arithmetic_expression(parser_t *parser, scope_t *scope) {
  AST_T *expression = init_ast(parser->tracker, AST_EXPRESSION);
  expression->scope = scope;

  for (;;) {
    if (expression->expression_size < 1) {
      expression->expression_values = calloc(1, sizeof(struct AST_STRUCT *));
    }

    if (parser->current_token->type != T_ID &&
        parser->current_token->type != T_ARITHMETIC_OPERATOR &&
        parser->current_token->type != T_INT &&
        parser->current_token->type != T_FLOAT) {
      break;
    } else {
      if (parser->current_token->type != T_ARITHMETIC_OPERATOR) {
        AST_T *ast_term;

        switch (parser->current_token->type) {
        case T_INT:
          ast_term = parse_int(parser, scope);
          break;
        case T_FLOAT:
          ast_term = parse_float(parser, scope);
          break;
        case T_ID:
          ast_term = parse_id(parser, scope);
          break;
        default: {
          log_error(parser->tracker, "expression member is neither an ID nor a"
                                     "number");
        }
        }

        if (ast_term) {
          expression->expression_size += 1;
          expression->expression_values = realloc(
              expression->expression_values,
              expression->expression_size * sizeof(struct AST_STRUCT *));
          expression->expression_values[expression->expression_size - 1] =
              ast_term;
        } else {
          log_error(parser->tracker,
                    "no term was gotten when parsing expression");
        }
      } else if (parser->current_token->type == T_ARITHMETIC_OPERATOR) {
        AST_T *ast_operator = parse_operator(parser, scope);

        if (ast_operator) {
          expression->expression_size += 1;
          expression->expression_values = realloc(
              expression->expression_values,
              expression->expression_size * sizeof(struct AST_STRUCT *));
          expression->expression_values[expression->expression_size - 1] =
              ast_operator;
        } else {
          log_error(parser->tracker, "no operator was returned from "
                                     "parser_parse_operator");
        }
      }
    }
  }
  return expression;
}

AST_T *parse_conditional_expression(parser_t *parser, scope_t *scope) {
  AST_T *conditional = init_ast(parser->tracker, AST_CONDITIONAL_EXPRESSION);
  conditional->scope = scope;

  for (;;) {
    if (conditional->conditional_expression_size < 1) {
      conditional->conditional_expression_values =
          calloc(1, sizeof(struct AST_STRUCT *));
    }

    if (parser->current_token->type == T_LBRACE) {
      break;
    } else {
      if (parser->current_token->type != T_CONDITIONAL_OPERATOR &&
          parser->current_token->type != T_LOGICAL_OPERATOR) {
        AST_T *ast_term = parse_term(parser, scope);

        if (ast_term) {
          conditional->conditional_expression_size += 1;
          conditional->conditional_expression_values =
              realloc(conditional->conditional_expression_values,
                      conditional->conditional_expression_size *
                          sizeof(struct AST_STRUCT *));
          conditional->conditional_expression_values
              [conditional->conditional_expression_size - 1] = ast_term;
        } else {
          log_error(parser->tracker,
                    "no term was gotten when parsing conditional "
                    "expression");
        }
      } else if (parser->current_token->type == T_CONDITIONAL_OPERATOR) {
        AST_T *ast_operator = parse_conditional_operator(parser, scope);

        if (ast_operator) {
          conditional->conditional_expression_size += 1;
          conditional->conditional_expression_values =
              realloc(conditional->conditional_expression_values,
                      conditional->conditional_expression_size *
                          sizeof(struct AST_STRUCT *));
          conditional->conditional_expression_values
              [conditional->conditional_expression_size - 1] = ast_operator;
        } else {
          log_error(parser->tracker, "no operator was returned from "
                                     "parse_conditional_operator");
        }
      } else if (parser->current_token->type == T_LOGICAL_OPERATOR) {
        AST_T *ast_operator = parse_logical_operator(parser, scope);

        if (ast_operator) {
          conditional->conditional_expression_size += 1;
          conditional->conditional_expression_values =
              realloc(conditional->conditional_expression_values,
                      conditional->conditional_expression_size *
                          sizeof(struct AST_STRUCT *));
          conditional->conditional_expression_values
              [conditional->conditional_expression_size - 1] = ast_operator;
        } else {
          log_error(parser->tracker, "no operator was returned from "
                                     "parse_logical_operator");
        }
      }
    }
  }

  if (conditional->conditional_expression_size == 0) {
    log_error(parser->tracker, "conditional expression has zero items!");
  }

  for (int i = 0; i < conditional->conditional_expression_size; i++) {
    AST_T *this_node = conditional->conditional_expression_values[i];

    // Check if the first item is a value
    if (i == 0 && (this_node->type == AST_CONDITIONAL_OPERATOR ||
                   this_node->type == AST_LOGICAL_OPERATOR)) {

      if (this_node->type == AST_LOGICAL_OPERATOR &&
          this_node->logical_operator_value == LO_NOT &&
          conditional->conditional_expression_size > 1) {
      } else {

        log_error(parser->tracker,
                  "first item in conditional expression must be a value");
      }
    }
    // Check if the last item is a value
    else if ((this_node->type == AST_LOGICAL_OPERATOR ||
              this_node->type == AST_CONDITIONAL_OPERATOR) &&
             i == conditional->conditional_expression_size - 1) {
      log_error(parser->tracker,
                "last item in conditional expression must be a value");
    }

    // there is only 1 element
    if (conditional->conditional_expression_size == 1) {
      break;
    }

    // conditional operator
    if (this_node->type == AST_CONDITIONAL_OPERATOR) {

      if (i != 0 && (conditional->conditional_expression_values[i - 1]->type ==
                         AST_CONDITIONAL_OPERATOR ||
                     conditional->conditional_expression_values[i - 1]->type ==
                         AST_LOGICAL_OPERATOR)) {
        log_error(parser->tracker,
                  "previous item before conditional operator must be "
                  "a value");
      }
      if (i + 1 != conditional->conditional_expression_size &&
              conditional->conditional_expression_values[i + 1]->type ==
                  AST_CONDITIONAL_OPERATOR ||
          conditional->conditional_expression_values[i + 1]->type ==
              AST_LOGICAL_OPERATOR) {
        log_error(parser->tracker,
                  "next item after conditional operator must be "
                  "a value");
      }
    }

    // logical operator
    if (this_node->type == AST_LOGICAL_OPERATOR) {
      if (i != 0 && (conditional->conditional_expression_values[i - 1]->type ==
                         AST_CONDITIONAL_OPERATOR ||
                     conditional->conditional_expression_values[i - 1]->type ==
                         AST_LOGICAL_OPERATOR)) {
        if (this_node->logical_operator_value != LO_NOT) {
          log_error(parser->tracker,
                    "previous item before logical operator must be "
                    "a value");
        }
      }

      // making a special case for NOT operator
      if (i != 0 && (conditional->conditional_expression_values[i - 1]->type !=
                         AST_CONDITIONAL_OPERATOR &&
                     conditional->conditional_expression_values[i - 1]->type !=
                         AST_LOGICAL_OPERATOR)) {
        if (this_node->logical_operator_value == LO_NOT) {
          log_error(parser->tracker,
                    "previous item before NOT operator must be "
                    "an operator");
        }
      }

      if (i + 1 != conditional->conditional_expression_size &&
              conditional->conditional_expression_values[i + 1]->type ==
                  AST_CONDITIONAL_OPERATOR ||
          conditional->conditional_expression_values[i + 1]->type ==
              AST_LOGICAL_OPERATOR) {
        if (conditional->conditional_expression_values[i + 1]->type ==
                AST_LOGICAL_OPERATOR &&
            conditional->conditional_expression_values[i + 1]
                    ->logical_operator_value == LO_NOT) {
        } else {
          log_error(parser->tracker,
                    "next item after logical operator must be a value");
        }
      }
    }

    // value
    if (this_node->type != AST_CONDITIONAL_OPERATOR &&
        this_node->type != AST_LOGICAL_OPERATOR) {

      if (i != 0 && (conditional->conditional_expression_values[i - 1]->type !=
                         AST_CONDITIONAL_OPERATOR &&
                     conditional->conditional_expression_values[i - 1]->type !=
                         AST_LOGICAL_OPERATOR)) {
        log_error(parser->tracker, "previous item before value must be "
                                   "an operator");
      }
      if (i + 1 != conditional->conditional_expression_size &&
          conditional->conditional_expression_values[i + 1]->type !=
              AST_CONDITIONAL_OPERATOR &&
          conditional->conditional_expression_values[i + 1]->type !=
              AST_LOGICAL_OPERATOR) {
        log_error(parser->tracker, "next item after value must be "
                                   "an operator");
      }
    }
  }

  return conditional;
}

AST_T *parse_id(parser_t *parser, scope_t *scope) {
  // Handle keywords first
  // handle keywords
  if (strcmp(parser->current_token->value, "let") == 0) {
    return parse_variable_definition(parser, scope);
  } else if (strcmp(parser->current_token->value, "fn") == 0) {
    return parse_function_definition(parser, scope);
  } else if (strcmp(parser->current_token->value, "if") == 0) {
    return parse_if_statement(parser, scope);
  } else if (strcmp(parser->current_token->value, "for") == 0) {
    return parse_for_loop(parser, scope);
  } else if (strcmp(parser->current_token->value, "loop") == 0) {
    return parse_loop_loop(parser, scope);
  } else if (strcmp(parser->current_token->value, "return") == 0) {
    return parse_return_statement(parser, scope);
  } else if (strcmp(parser->current_token->value, "break") == 0) {
    return parse_break_statement(parser, scope);
  } else if (strcmp(parser->current_token->value, "continue") == 0) {
    return parse_continue_statement(parser, scope);
  } else if (strcmp(parser->current_token->value, "use") == 0) {
    return parse_use_statement(parser, scope);
  } else {
    parser_expect(parser, T_ID); // variable or function call name

    int is_function = parser_expect_check(parser, T_LPAREN);
    int is_redefinition = parser_expect_check(parser, T_EQUALS);

    if (is_function) {
      return parse_function_call(parser, scope);
    } else if (is_redefinition) {
      return parse_variable_redefinition(parser, scope);
    } else {
      return parse_variable(parser, scope);
    }
  }
}

AST_T *parse_use_statement(parser_t *parser, scope_t *scope) {
  parser_expect(parser, T_ID); // expect use keyword

  AST_T *use_statement = init_ast(parser->tracker, AST_USE_STATEMENT);

  char *file_path = calloc(1, sizeof(char));

  char *function_name = calloc(1, sizeof(char));

  int depth = 0;

  for (;;) {
    if (parser->current_token->type == T_SEMICOLON) {
      break;
    }

    if (depth == 0) {
      parser_expect(parser, T_ID);

      file_path =
          realloc(file_path,
                  (strlen("./") + 1) +
                      (strlen(parser->prev_token->value) + 1) * sizeof(char));
      strcpy(file_path, "./");
      strcat(file_path, parser->prev_token->value);

      parser_expect(parser, T_COLON);
      parser_expect(parser, T_COLON);

      if (parser->current_token->type == T_ARITHMETIC_OPERATOR &&
          strcmp(parser->current_token->value, "*") == 0) {

        function_name =
            realloc(function_name,
                    (strlen(parser->current_token->value) + 1) * sizeof(char));
        strcpy(function_name, parser->current_token->value);
        parser_expect(parser, T_ARITHMETIC_OPERATOR);

      } else {
        parser_expect(parser, T_ID);

        function_name =
            realloc(function_name,
                    (strlen(parser->prev_token->value) + 1) * sizeof(char));
        strcpy(function_name, parser->prev_token->value);
      }

      depth++;
    } else {
      file_path =
          realloc(file_path, (strlen(file_path) + 1) + (strlen("/") + 1) +
                                 (strlen(function_name) + 1) * sizeof(char));

      strcat(file_path, "/");
      strcat(file_path, function_name);

      parser_expect(parser, T_COLON);
      parser_expect(parser, T_COLON);

      if (parser->current_token->type == T_ARITHMETIC_OPERATOR &&
          strcmp(parser->current_token->value, "*") == 0) {

        function_name =
            realloc(function_name,
                    (strlen(parser->current_token->value) + 1) * sizeof(char));
        strcpy(function_name, parser->current_token->value);
        parser_expect(parser, T_ARITHMETIC_OPERATOR);

      } else {
        parser_expect(parser, T_ID);

        function_name =
            realloc(function_name,
                    (strlen(parser->prev_token->value) + 1) * sizeof(char));
        strcpy(function_name, parser->prev_token->value);
      }

      if (parser->current_token->type != T_SEMICOLON) {
        depth++;
      }
    }
  }

  use_statement->scope = scope;
  use_statement->use_file_path = file_path;
  use_statement->use_function_name = function_name;

  return use_statement;
}

AST_T *parse_break_statement(parser_t *parser, scope_t *scope) {
  parser_expect(parser, T_ID); // expect break keyword

  AST_T *break_statement = init_ast(parser->tracker, AST_BREAK_STATEMENT);

  break_statement->scope = scope;

  return break_statement;
}

AST_T *parse_continue_statement(parser_t *parser, scope_t *scope) {
  parser_expect(parser, T_ID); // expect continue keyword

  AST_T *continue_statement = init_ast(parser->tracker, AST_CONTINUE_STATEMENT);

  continue_statement->scope = scope;

  return continue_statement;
}

AST_T *parse_for_loop(parser_t *parser, scope_t *scope) {
  AST_T *ast = init_ast(parser->tracker, AST_FOR_LOOP);
  ast->scope = scope;

  parser_expect(parser, T_ID); // expect the for keyword

  ast->for_loop_counter = parse_id(parser, scope);

  parser_expect(parser, T_ID); // expect the in keyword

  parser_expect(parser, T_NUMBER_RANGE);
  ast->for_loop_range = parse_number_range(parser, scope);

  scope_t *new_scope = init_scope();
  ast->for_loop_body = parse_block(parser, scope, new_scope, BLOCK_FOR);

  return ast;
}

AST_T *parse_loop_loop(parser_t *parser, scope_t *scope) {
  AST_T *ast = init_ast(parser->tracker, AST_LOOP_LOOP);
  ast->scope = scope;

  parser_expect(parser, T_ID); // expect the loop keyword

  scope_t *new_scope = init_scope();
  ast->loop_loop_body = parse_block(parser, scope, new_scope, BLOCK_LOOP);

  return ast;
}

AST_T *parse_if_statement(parser_t *parser, scope_t *scope) {
  AST_T *ast = init_ast(parser->tracker, AST_IF_STATEMENT);
  ast->scope = scope;

  parser_expect(parser, T_ID); // expect the if keyword

  ast->if_statement_condition = parse_conditional_expression(parser, scope);

  scope_t *new_scope = init_scope();
  ast->if_statement_body = parse_block(parser, scope, new_scope, BLOCK_IF);

  for (;;) {
    if (parser->current_token->type == T_ID &&
        strcmp(parser->current_token->value, "else") == 0) {
      parser_expect(parser, T_ID);

      if (parser->current_token->type == T_ID &&
          strcmp(parser->current_token->value, "if") == 0) {
        parser_expect(parser, T_ID);

        if (ast->else_if_count < 1) {
          ast->else_if_values = calloc(1, sizeof(struct AST_STRUCT *));
        }

        ast->else_if_count++;
        // construct an if statement AST node

        AST_T *else_if_ast = init_ast(parser->tracker, AST_IF_STATEMENT);
        else_if_ast->scope = scope;

        else_if_ast->if_statement_condition =
            parse_conditional_expression(parser, scope);
        scope_t *new_scope = init_scope();
        else_if_ast->if_statement_body =
            parse_block(parser, scope, new_scope, BLOCK_IF);

        // add it to the else_if_values

        ast->else_if_values =
            realloc(ast->else_if_values,
                    ast->else_if_count * sizeof(struct AST_STRUCT *));
        ast->else_if_values[ast->else_if_count - 1] = else_if_ast;
      } else {
        ast->has_else = 1;
        scope_t *new_scope = init_scope();
        ast->else_body = parse_block(parser, scope, new_scope, BLOCK_IF);
      }
    } else {
      break;
    }
  }

  return ast;
}

void add_node_to_tail(AST_T *node, AST_T *parent) {
  int new_size = parent->block_size + 1;

  AST_T **new_block_value = (AST_T **)malloc(new_size * sizeof(AST_T *));
  if (parent->block_size == 1) {
    new_block_value[0] = parent->block_values[0];
    new_block_value[1] = node;
  } else {
    for (int i = 0; i < parent->block_size; i++)
      new_block_value[i] = parent->block_values[i];
    new_block_value[parent->block_size] = node;
  }

  free(parent->block_values);
  parent->block_size = new_size;
  parent->block_values = new_block_value;
}

void add_node_to_head(AST_T *node, AST_T *parent) {
  int new_size = parent->block_size + 1;

  AST_T **new_block_value = (AST_T **)malloc(new_size * sizeof(AST_T *));
  new_block_value[0] = node;

  for (int i = 0; i < parent->block_size; i++) {
    new_block_value[i + 1] = parent->block_values[i];
  }

  free(parent->block_values);
  parent->block_size = new_size;
  parent->block_values = new_block_value;
}