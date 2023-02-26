#include "../include/libchad/libchad.h"
#include "../include/scope/scope.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

// #include <stdio.h>
#include <string.h>

visitor_t *init_visitor() {
  visitor_t *visitor = calloc(1, sizeof(struct VISITOR_STRUCT));
  visitor->tracker = (void *)0;

  visitor->returned = 0;
  visitor->loop_break = 0;
  visitor->loop_continue = 0;

  return visitor;
}

AST_T *visit(visitor_t *visitor, AST_T *node) {
  visitor->tracker->line = node->line;
  visitor->tracker->column = node->column;

  switch (node->type) {
  case AST_VARIABLE_DEFINITION:
    return visit_variable_definition(visitor, node);
    break;
  case AST_VARIABLE_REDEFINITION:
    return visit_variable_redefinition(visitor, node);
    break;
  case AST_FUNCTION_DEFINITION:
    return visit_function_definition(visitor, node);
    break;

  case AST_RETURN_STATEMENT:
    return visit_return_statement(visitor, node);
    break;

  case AST_BREAK_STATEMENT:
    return visit_break_statement(visitor, node);
    break;

  case AST_CONTINUE_STATEMENT:
    return visit_continue_statement(visitor, node);
    break;

  case AST_USE_STATEMENT:
    return visit_use_statement(visitor, node);
    break;

  case AST_VARIABLE:
    return visit_variable(visitor, node);
    break;
  case AST_EXPRESSION:
    return visit_arithmetic_expression(visitor, node);
    break;
  case AST_CONDITIONAL_EXPRESSION:
    return visit_conditional_expression(visitor, node);
    break;
  case AST_FUNCTION_CALL:
    return visit_function_call(visitor, node);
    break;
  case AST_STRING:
    return visit_int(visitor, node);
    break;
  case AST_INT:
    return visit_string(visitor, node);
    break;
  case AST_CHAR:
    return visit_char(visitor, node);
    break;
  case AST_FLOAT:
    return visit_float(visitor, node);
    break;
  case AST_BOOL:
    return visit_bool(visitor, node);
    break;
  case AST_ARITHMETIC_OPERATOR:
    return visit_operator(visitor, node);
    break;
  case AST_CONDITIONAL_OPERATOR:
    return visit_conditional_operator(visitor, node);
    break;
  case AST_LOGICAL_OPERATOR:
    return visit_logical_operator(visitor, node);
    break;
  case AST_BLOCK:
    return visit_block(visitor, node);
    break;
  case AST_IF_STATEMENT:
    return visit_if_statement(visitor, node);
    break;
  case AST_FOR_LOOP:
    return visit_for_loop(visitor, node);
    break;
  case AST_LOOP_LOOP:
    return visit_loop_loop(visitor, node);
    break;

  case AST_NULL:
    log_error(visitor->tracker, "encountered a NULL node");

    break;
  case AST_EOF:
    log_print("RUNTIME - reached EOF. stopping execution....");
    exit(0);
  }

  log_error(visitor->tracker, "uncaught statement of type `%d`", node->type);
}

AST_T *visit_break_statement(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_use_statement(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_continue_statement(visitor_t *visitor, AST_T *node) {
  return node;
}

AST_T *visit_if_statement(visitor_t *visitor, AST_T *node) {
  AST_T *condition = visit(visitor, node->if_statement_condition);

  if (condition->bool_value == true) {
    return visit(visitor, node->if_statement_body);
  } else {

    if (node->else_if_count != 0) {
      for (int i = 0; i < node->else_if_count; i++) {
        AST_T *current_else_if = node->else_if_values[i];

        AST_T *current_condition =
            visit(visitor, current_else_if->if_statement_condition);

        if (current_condition->bool_value == true) {
          return visit(visitor, current_else_if->if_statement_body);
        }
      }
    }

    if (node->has_else) {
      return visit(visitor, node->else_body);
    }
  }

  return node;
}

AST_T *visit_for_loop(visitor_t *visitor, AST_T *node) {
  // craft a new variable definition for the counter
  AST_T *vdef = init_ast(visitor->tracker, AST_VARIABLE_DEFINITION);

  char *counter_name = node->for_loop_counter->variable_name;

  AST_T *start_value = visit(visitor, node->for_loop_range->number_range_start);

  AST_T *end_value = visit(visitor, node->for_loop_range->number_range_end);

  if (start_value->type != AST_INT || end_value->type != AST_INT) {
    log_error(visitor->tracker, "for-loop range contains non integer values");
  }

  scope_t *counter_scope = node->for_loop_body->scope;

  ast_type_t counter_type = AST_INT;

  vdef->variable_definition_variable_name = counter_name;
  vdef->variable_definition_type = counter_type;
  vdef->variable_definition_value = init_ast(visitor->tracker, AST_INT);
  vdef->variable_definition_value->int_value = start_value->int_value;
  vdef->scope = counter_scope;

  scope_add_variable_definition(visitor->tracker, vdef->scope, vdef);

  for (;;) {

    if (visitor->loop_break) {
      visitor->loop_break = 0;

      break;
    }

    if (visitor->loop_continue) {
      visitor->loop_continue = 0;
    }

    AST_T *counter_value = scope_get_variable_definition(
                               visitor->tracker, counter_scope, counter_name)
                               ->variable_definition_value;

    if (counter_value->int_value < end_value->int_value &&
        node->for_loop_body->scope->loop_break != 1) {

      visit(visitor, node->for_loop_body);

      counter_value->int_value++;
    } else {
      node->for_loop_body->scope->loop_break = 0;
      node->for_loop_body->scope->loop_continue = 0;
      break;
    }
  }

  return node;
}

// BUG: using return in a loop loop doesn't work
AST_T *visit_loop_loop(visitor_t *visitor, AST_T *node) {
  for (;;) {

    if (visitor->loop_break) {
      visitor->loop_break = 0;

      break;
    }

    if (visitor->loop_continue) {
      visitor->loop_continue = 0;
    }

    visit(visitor, node->loop_loop_body);
  }

  return node;
}

AST_T *visit_variable_definition(visitor_t *visitor, AST_T *node) {

  node->variable_definition_value =
      visit(visitor, node->variable_definition_value);

  scope_add_variable_definition(visitor->tracker, node->scope, node);

  return node;
}

AST_T *visit_variable_redefinition(visitor_t *visitor, AST_T *redef) {
  AST_T *vdef =
      scope_get_variable_definition(visitor->tracker, redef->scope,
                                    redef->variable_redefinition_variable_name);

  if ((vdef->variable_definition_type == redef->variable_redefinition_type) ||
      (vdef->variable_definition_type == AST_EXPRESSION &&
       redef->variable_redefinition_type == AST_INT) ||
      (vdef->variable_definition_type == AST_INT &&
       redef->variable_redefinition_type == AST_EXPRESSION) ||
      redef->variable_redefinition_type == AST_FUNCTION_CALL) {

    vdef->variable_definition_value = visit(visitor, redef->variable_redefinition_value);
    vdef->variable_definition_type = redef->variable_redefinition_type;
  } else {
    log_error(
        visitor->tracker,
        "cannot redefine variable %s with a different "
        "type. expected %s, found %s",
        redef->variable_redefinition_variable_name,
        ast_type_to_str(visitor->tracker, vdef->variable_definition_type),
        ast_type_to_str(visitor->tracker, redef->variable_redefinition_type));
  }

  return redef;
}

int values_equal(visitor_t *visitor, AST_T *left, AST_T *right) {
  if (left->type != right->type) {
    log_error(visitor->tracker, "values are not of the same type, checking for "
                                "equality");
  }

  if (left->type == AST_INT) {
    if (left->int_value == right->int_value) {
      return 1;
    }
  }

  if (left->type == AST_CHAR) {
    if (left->char_value == right->char_value) {
      return 1;
    }
  }

  if (left->type == AST_STRING) {
    if (strcmp(left->string_value, right->string_value) == 0) {
      return 1;
    }
  }

  if (left->type == AST_FLOAT) {
    if (left->float_value == right->float_value) {
      return 1;
    }
  }

  if (left->type == AST_BOOL) {
    if (left->bool_value == right->bool_value) {
      return 1;
    }
  }

  return 0;
}

int values_greater(visitor_t *visitor, AST_T *left, AST_T *right) {
  if (left->type != AST_INT && left->type != AST_FLOAT) {
    log_error(visitor->tracker, "left value is not a number, checking for "
                                "greater-than");
  }

  if (right->type != AST_INT && right->type != AST_FLOAT) {
    log_error(visitor->tracker, "right value is not a number, checking for "
                                "greater-than");
  }

  float left_value = 0;

  if (left->type == AST_INT) {
    left_value = left->int_value;
  } else if (left->type == AST_FLOAT) {
    left_value = left->float_value;
  }

  float right_value = 0;

  if (right->type == AST_INT) {
    right_value = right->int_value;
  } else if (right->type == AST_FLOAT) {
    right_value = right->float_value;
  }

  if (left_value > right_value) {
    return 1;
  }

  return 0;
}

int values_less(visitor_t *visitor, AST_T *left, AST_T *right) {
  if (left->type != AST_INT && left->type != AST_FLOAT) {
    log_error(visitor->tracker, "left value is not a number, checking for "
                                "less-than");
  }

  if (right->type != AST_INT && right->type != AST_FLOAT) {
    log_error(visitor->tracker, "right value is not a number, checking for "
                                "less-than");
  }

  float left_value = 0;

  if (left->type == AST_INT) {
    left_value = left->int_value;
  } else if (left->type == AST_FLOAT) {
    left_value = left->float_value;
  }

  float right_value = 0;

  if (right->type == AST_INT) {
    right_value = right->int_value;
  } else if (right->type == AST_FLOAT) {
    right_value = right->float_value;
  }

  if (left_value < right_value) {
    return 1;
  }

  return 0;
}

int values_greater_or_equal(visitor_t *visitor, AST_T *left, AST_T *right) {
  if (left->type != AST_INT && left->type != AST_FLOAT) {
    log_error(visitor->tracker, "left value is not a number, checking for "
                                "greater or equal");
  }

  if (right->type != AST_INT && right->type != AST_FLOAT) {
    log_error(visitor->tracker, "right value is not a number, checking for "
                                "greater or equal");
  }

  float left_value = 0;

  if (left->type == AST_INT) {
    left_value = left->int_value;
  } else if (left->type == AST_FLOAT) {
    left_value = left->float_value;
  }

  float right_value = 0;

  if (right->type == AST_INT) {
    right_value = right->int_value;
  } else if (right->type == AST_FLOAT) {
    right_value = right->float_value;
  }

  if (left_value >= right_value) {
    return 1;
  }

  return 0;
}

int values_less_or_equal(visitor_t *visitor, AST_T *left, AST_T *right) {
  if (left->type != AST_INT && left->type != AST_FLOAT) {
    log_error(visitor->tracker, "left value is not a number, checking for "
                                "less or equal");
  }

  if (right->type != AST_INT && right->type != AST_FLOAT) {
    log_error(visitor->tracker, "right value is not a number, checking for "
                                "less or equal");
  }

  float left_value = 0;

  if (left->type == AST_INT) {
    left_value = left->int_value;
  } else if (left->type == AST_FLOAT) {
    left_value = left->float_value;
  }

  float right_value = 0;

  if (right->type == AST_INT) {
    right_value = right->int_value;
  } else if (right->type == AST_FLOAT) {
    right_value = right->float_value;
  }

  if (left_value <= right_value) {
    return 1;
  }

  return 0;
}

AST_T *visit_conditional_expression(visitor_t *visitor, AST_T *node) {
  bool result_buffer = false;
  logical_operator_t current_logical_operator = LO_NONE;

  // iterating through every item in the expression
  for (int i = 0; i < node->conditional_expression_size; i++) {
    AST_T *this_node = node->conditional_expression_values[i]; // current item

    AST_T *final = visit(visitor, this_node);

    if (i == 0 && node->conditional_expression_size == 1) {
      if (final->type == AST_BOOL) {
        result_buffer = final->bool_value;
        continue;
      } else if (final->type == AST_INT) {
        if (final->int_value == 0) {
          result_buffer = false;
          continue;
        } else {
          result_buffer = true;
          continue;
        }
      } else {
        log_error(visitor->tracker,
                  "single condition is neither a boolean nor "
                  "integer value. found %d",
                  final->type);
      }
    }

    if (final->type == AST_LOGICAL_OPERATOR) {
      if (final->logical_operator_value == LO_AND) {
        current_logical_operator = LO_AND;
        continue;
      } else if (final->logical_operator_value == LO_OR) {
        current_logical_operator = LO_OR;
        continue;
      } else if (final->logical_operator_value == LO_NOT) {
        current_logical_operator = LO_NOT;
        continue;
      }
    }

    if (i + 1 == node->conditional_expression_size) {
      bool buffer = false;

      if (final->type == AST_BOOL) {
        buffer = final->bool_value;
      } else if (final->type == AST_INT) {
        if (final->int_value == 0) {
          buffer = false;
        } else {
          buffer = true;
        }
      } else {
        log_error(visitor->tracker,
                  "single condition is neither a boolean nor "
                  "integer value. found %d",
                  final->type);
      }

      result_buffer = apply_logical_operator(result_buffer, buffer,
                                             current_logical_operator);
      continue;
    }

    AST_T *next_node = node->conditional_expression_values[i + 1]; // next item
    AST_T *next_final = visit(visitor, next_node);

    if (next_final->type == AST_LOGICAL_OPERATOR) {
      bool buffer = false;

      if (final->type == AST_BOOL) {
        buffer = final->bool_value;
      } else if (final->type == AST_INT) {
        if (final->int_value == 0) {
          buffer = false;
        } else {
          buffer = true;
        }
      } else {
        log_error(visitor->tracker,
                  "single item is neither a boolean nor "
                  "integer value. found %d",
                  final->type);
      }

      result_buffer = apply_logical_operator(result_buffer, buffer,
                                             current_logical_operator);
      continue;
    }

    AST_T *next_next_node =
        node->conditional_expression_values[i + 2]; // next next item, LOL
    AST_T *next_next_final = visit(visitor, next_next_node);

    bool buffer = false;
    int modified = 0;

    switch (next_final->conditional_operator_value) {
    case CON_GT:
      buffer = values_greater(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    case CON_LT:
      buffer = values_less(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    case CON_EQ:
      buffer = values_equal(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    case CON_NEQ:
      buffer = !values_equal(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    case CON_GTE:
      buffer = values_greater_or_equal(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    case CON_LTE:
      buffer = values_less_or_equal(visitor, final, next_next_final);
      modified = 1;
      i += 2;
      break;
    }

    if (modified) {
      result_buffer = apply_logical_operator(result_buffer, buffer,
                                             current_logical_operator);
    }
  }

  AST_T *result = malloc(sizeof(AST_T));
  result->type = AST_BOOL;
  result->bool_value = result_buffer;
  return result;
}

bool apply_logical_operator(bool result_buffer, bool buffer,
                            logical_operator_t current_logical_operator) {

  if (current_logical_operator == LO_AND) {
    return result_buffer &= buffer;
  } else if (current_logical_operator == LO_OR) {
    return result_buffer |= buffer;
  } else if (current_logical_operator == LO_NOT) {
    return !buffer;
  } else {
    return buffer;
  }
}

// TODO: implement precedence for different operators and parentheses
// NOTE: Arithmetic expressions will always evaluate to an integer and floats
// will be rounded up
AST_T *visit_arithmetic_expression(visitor_t *visitor, AST_T *node) {
  float result_buffer = 0;
  operator_t current_operator = O_NONE;

  // iterating through every item in the expression
  for (int i = 0; i < node->expression_size; i++) {
    AST_T *this_node = node->expression_values[i]; // current item

    AST_T *eventually = visit(visitor, this_node);

    if (eventually->type != AST_INT && eventually->type != AST_FLOAT &&
        eventually->type != AST_ARITHMETIC_OPERATOR) {
      log_error(visitor->tracker,
                "cannot use a non-number type in an arithmetic "
                "expression. "
                "found %s",
                ast_type_to_str(visitor->tracker, eventually->type));
    }

    if (i == 0 && eventually->type != AST_INT &&
        eventually->type != AST_FLOAT) {
      log_error(visitor->tracker, "first item in expression must be a number");
    }

    if (i == 0) {
      if (eventually->type == AST_INT) {
        result_buffer = eventually->int_value;
      } else if (eventually->type == AST_FLOAT) {
        result_buffer = eventually->float_value;
      }
      continue;
    }

    if (eventually->type == AST_INT) {
      switch (current_operator) {
      case O_ADD:
        result_buffer += eventually->int_value;
        break;
      case O_SUB:
        result_buffer -= eventually->int_value;
        break;
      case O_MUL:
        result_buffer *= eventually->int_value;
        break;
      case O_DIV:
        result_buffer /= eventually->int_value;
        break;
      }
    } else if (eventually->type == AST_FLOAT) {
      switch (current_operator) {
      case O_ADD:
        result_buffer += eventually->float_value;
        break;
      case O_SUB:
        result_buffer -= eventually->float_value;
        break;
      case O_MUL:
        result_buffer *= eventually->float_value;
        break;
      case O_DIV:
        result_buffer /= eventually->float_value;
        break;
      }

    } else if (eventually->type == AST_ARITHMETIC_OPERATOR) {
      current_operator = eventually->operator_value;
    }
  }

  AST_T *final_result = init_ast(visitor->tracker, AST_INT);
  final_result->int_value = result_buffer;

  return final_result;
}

AST_T *visit_variable(visitor_t *visitor, AST_T *node) {
  AST_T *vdef = scope_get_variable_definition(visitor->tracker, node->scope,
                                              node->variable_name);

  if (vdef != (void *)0) {
    AST_T *eventually = visit(visitor, vdef->variable_definition_value);

    if (eventually->type == AST_BLOCK) {
      AST_T *block_eventually =
          visit(visitor, eventually->scope->returned_value);

      if (vdef->variable_definition_type != block_eventually->type &&
          vdef->variable_definition_type != AST_FUNCTION_CALL &&
          vdef->variable_definition_type != AST_VARIABLE) {
        // enforce type checking
        log_error(
            visitor->tracker,
            "variable `%s` with type %s cannot be assigned "
            "to a function that returns value of type %s",
            vdef->variable_definition_variable_name,
            ast_type_to_str(visitor->tracker, vdef->variable_definition_type),
            ast_type_to_str(visitor->tracker, block_eventually->type));
      }

      return block_eventually;
    }

    if (vdef->variable_definition_type != eventually->type &&
        vdef->variable_definition_type != AST_FUNCTION_CALL &&
        vdef->variable_definition_type != AST_VARIABLE &&
        vdef->variable_definition_type != AST_EXPRESSION) {
      // enforce type checking
      log_error(
          visitor->tracker,
          "variable `%s` with type %s cannot be assigned to a "
          "value of type %s\n",
          vdef->variable_definition_variable_name,
          ast_type_to_str(visitor->tracker, vdef->variable_definition_type),
          ast_type_to_str(visitor->tracker, eventually->type));
    }

    return eventually;
  } else {
    log_error(visitor->tracker, "undefined variable `%s`", node->variable_name);
  }
}
