#include "../include/libchad/libchad.h"
#include "../include/scope/scope.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <stdio.h>
#include <string.h>

AST_T *visit_function_definition(visitor_t *visitor, AST_T *node) {
  scope_add_function_definition(visitor->tracker, node->scope, node);

  return node;
}

// AST_T *visit_return_statement(visitor_t *visitor, AST_T *node) { return node; }


// BUG: function add_numbers chad example returns invalid sums
AST_T *visit_function_call(visitor_t *visitor, AST_T *node) {

  AST_T *fdef = scope_get_function_definition(visitor->tracker, node->scope,
                                              node->function_call_name);

  if (fdef == (void *)0) {
    log_error(visitor->tracker, "undefined function `%s`",
              node->function_call_name);
  }

  if (node->function_call_arguments_size <
          fdef->function_definition_args_size &&
      !fdef->function_definition_is_variadic) {
    log_error(visitor->tracker,
              "too few parameters were passed to function `%s`",
              node->function_call_name);
  }

  if (node->function_call_arguments_size >
          fdef->function_definition_args_size &&
      !fdef->function_definition_is_variadic) {
    log_error(visitor->tracker,
              "too many parameters were passed to function `%s`",
              node->function_call_name);
  }

  for (int i = 0; i < (int)node->function_call_arguments_size; i++) {

    if (!fdef->function_definition_is_variadic) {
      // grab the variable from the function definition arguments
      AST_T *ast_var = (AST_T *)fdef->function_definition_args[i];

      // grab the value from the function call arguments
      AST_T *ast_value = visit(visitor, node->function_call_arguments[i]);

      // create a new variable definition with the value of the argument
      // in the function call.
      AST_T *ast_vardef = init_ast(visitor->tracker, AST_VARIABLE_DEFINITION);
      ast_vardef->variable_definition_value = ast_value;

      // BUG: getting error if func parameter type is not specified
      if (ast_var->type != ast_value->type && ast_var->type != AST_VARIABLE) {
        log_error(visitor->tracker,
                  "invalid type passed to function `%s`. expected %s "
                  "found %s",
                  node->function_call_name,
                  ast_type_to_str(visitor->tracker, ast_var->type),
                  ast_type_to_str(visitor->tracker, ast_value->type));
      }

      ast_vardef->variable_definition_type = ast_value->type;

      // copy the name from the function definition argument into the new
      // variable definition
      ast_vardef->variable_definition_variable_name =
          (char *)calloc((strlen(ast_var->variable_name) + 1), sizeof(char));
      strcpy(ast_vardef->variable_definition_variable_name,
             ast_var->variable_name);

      // push our variable definition into the function body scope.
      scope_add_variable_definition(
          visitor->tracker, fdef->function_definition_body->scope, ast_vardef);
    } else {

      // grab the value from the function call arguments
      AST_T *ast_value = (AST_T *)node->function_call_arguments[i];

      // create a new variable definition with the value of the argument
      // in the function call.
      AST_T *ast_vardef = init_ast(visitor->tracker, AST_VARIABLE_DEFINITION);
      ast_vardef->variable_definition_value = ast_value;

      ast_vardef->variable_definition_type = ast_value->type;

      char index_str[1024];
      sprintf(index_str, "%d", i);
      char *variadic_name = "__vars__";

      // copy the name from the function definition argument into the new
      // variable definition
      ast_vardef->variable_definition_variable_name = (char *)calloc(
          (strlen(variadic_name) + 1) + (strlen(index_str) + 1), sizeof(char));
      strcpy(ast_vardef->variable_definition_variable_name, variadic_name);
      strcat(ast_vardef->variable_definition_variable_name, index_str);

      // push our variable definition into the function body scope.
      // log_print("fn is: %s", fdef->function_definition_name);
      scope_add_variable_definition(
          visitor->tracker, fdef->function_definition_body->scope, ast_vardef);

      fdef->function_definition_args_size++;

      if (i == node->function_call_arguments_size - 1) {
        // create a new variable definition that holds the size of the variadic
        // function
        AST_T *size_value = init_ast(visitor->tracker, AST_INT);
        size_value->int_value = fdef->function_definition_args_size;

        AST_T *ast_size = init_ast(visitor->tracker, AST_VARIABLE_DEFINITION);
        ast_size->variable_definition_value = size_value;

        ast_size->variable_definition_type = AST_INT;

        char *size_name = "__vars__size";

        ast_size->variable_definition_variable_name =
            (char *)calloc((strlen(size_name) + 1), sizeof(char));
        strcpy(ast_size->variable_definition_variable_name, size_name);

        // push our variable definition into the function body scope.
        scope_add_variable_definition(
            visitor->tracker, fdef->function_definition_body->scope, ast_size);

        fdef->function_definition_args_size++;
      }
    }
  }

  AST_T *return_value;

  if (is_core_function(node->function_call_name)) {
    return_value = visit_core_function(visitor, fdef, node->function_call_name);
  } else {
    return_value = visit(visitor, fdef->function_definition_body);
  }

  clear_scope(fdef->function_definition_body->scope);
  if (fdef->function_definition_is_variadic) {
    fdef->function_definition_args_size = 0;
  }

  return return_value;
}

AST_T *visit_core_function(visitor_t *visitor, AST_T *fdef,
                           char *function_name) {
  if (strcmp(function_name, "print") == 0) {
    return core_print(visitor, fdef);
  } else if (strcmp(function_name, "println") == 0) {
    return core_println(visitor, fdef);
  } else if (strcmp(function_name, "exit") == 0) {
    return core_exit(visitor, fdef);
  } else if (strcmp(function_name, "read_file_to_string") == 0) {
    return core_read_file_to_string(visitor, fdef);
  } else if (strcmp(function_name, "write_string_to_file") == 0) {
    return core_write_string_to_file(visitor, fdef);
  } else if (strcmp(function_name, "pwd") == 0) {
    return core_pwd(visitor, fdef);
  } else if (strcmp(function_name, "get_timestamp") == 0) {
    return core_get_timestamp(visitor, fdef);
  } else if (strcmp(function_name, "cmd") == 0) {
    return core_cmd(visitor, fdef);
  } else if (strcmp(function_name, "chad_run") == 0) {
    return core_chad_run(visitor, fdef);
  } else if (strcmp(function_name, "http_get") == 0) {
    return core_http_get(visitor, fdef);
  } else if (strcmp(function_name, "strcat") == 0) {
    return core_strcat(visitor, fdef);
  } else {
    log_error(visitor->tracker, "core function `%s` is not implemented",
              function_name);
  }

  return fdef;
}

AST_T *visit_block(visitor_t *visitor, AST_T *node) {
  // iterating through every statement in the block
  for (int i = 0; i < node->block_size; i++) {

    if (visitor->returned) {
      break;
    }

    if (visitor->loop_break) {
      break;
    }

    if (visitor->loop_continue) {
      break;
    }

    node->current_block_item = i;

    AST_T *this_node = node->block_values[i];

    // if this is a function return statement
    if (this_node->type == AST_RETURN_STATEMENT) {
      // if the function returned a value
      if (this_node->returned_value != NULL) {
        AST_T *returned_value = visit(visitor, this_node->returned_value);

        // override the type if it is a variable
        if (node->scope->return_type == AST_VARIABLE) {
          node->scope->return_type = returned_value->type;
        }

        if (node->scope->return_type != AST_NULL &&
            returned_value->type != node->scope->return_type) {
          log_error(
              visitor->tracker,
              "RUNTIME - Return type does not match returned value. "
              "Expected "
              "%s found %s",
              ast_type_to_str(visitor->tracker, this_node->scope->return_type),
              ast_type_to_str(visitor->tracker, returned_value->type));
        }

        node->scope->returned_value = returned_value;

        visitor->returned = 1;
        break;
      } else {
        node->scope->returned_value = init_ast(visitor->tracker, AST_NULL);

        visitor->returned = 1;

        break;
      }

    } else if (this_node->type == AST_BREAK_STATEMENT) {
      visitor->loop_break = 1;

      break;
    } else if (this_node->type == AST_CONTINUE_STATEMENT) {
      visitor->loop_continue = 1;

      break;
    } else {
      visit(visitor, this_node);
    }
  }

  if (visitor->returned) {
    if (node->block_type == BLOCK_FN) {

      visitor->returned = 0;

      return node->scope->returned_value;

    } else {
      node->scope->parent_scope->returned_value = node->scope->returned_value;
      return node;
    }
  }

  return node;
}