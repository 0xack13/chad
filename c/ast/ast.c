#include "../include/utils/utils.h"


AST_T *init_ast(tracker_t *tracker, int type) {
  AST_T *ast = calloc(1, sizeof(struct AST_STRUCT));
  ast->type = type;

  // META
  ast->line = tracker->line;
  ast->column = tracker->column;

  ast->scope = (void *)0;

  ast->parent_block = (void *)0;

  // VARIABLE DEFINITION
  ast->variable_definition_variable_name = (void *)0;
  ast->variable_definition_value = (void *)0;
  ast->variable_definition_type = AST_NULL;

  // VARIABLE DEFINITION
  ast->variable_redefinition_variable_name = (void *)0;
  ast->variable_redefinition_value = (void *)0;
  ast->variable_redefinition_type = AST_NULL;

  // FUNCTION DEFINITION
  ast->function_definition_body = (void *)0;
  ast->function_definition_name = (void *)0;
  ast->function_definition_args = (void *)0;
  ast->function_definition_args_size = 0;
  ast->function_definition_is_variadic = 0;

  // VARIABLE
  ast->variable_name = (void *)0;

  // FUNCTION CALL
  ast->function_call_name = (void *)0;
  ast->function_call_arguments = (void *)0;
  ast->function_call_arguments_size = 0;

  // ACTUAL VALUES
  ast->string_value = (void *)0;
  ast->char_value = NULL_CHAR;
  ast->int_value = 0;
  ast->bool_value = false;
  ast->float_value = 0;
  ast->number_range_start = 0;
  ast->number_range_end = 0;

  // ARITHMETIC OPERATOR
  ast->operator_value = O_NONE;

  // CONDITIONAL OPERATOR
  ast->conditional_operator_value = CON_NONE;

  // LOGICAL OPERATOR
  ast->logical_operator_value = LO_NONE;

  // BLOCK
  ast->block_values = (void *)0;
  ast->block_size = 0;
  ast->block_type = BLOCK_NONE;
  ast->current_block_item = 0;
  ast->main_block = 0;

  // EXPRESSION
  ast->expression_values = (void *)0;
  ast->expression_size = 0;

  // CONDITIONAL EXPRESSION
  ast->conditional_expression_values = (void *)0;
  ast->conditional_expression_size = 0;

  // IF STATEMENT
  ast->if_statement_condition = (void *)0;
  ast->if_statement_body = (void *)0;
  ast->has_else = 0;
  ast->else_body = (void *)0;
  ast->else_if_count = 0;
  ast->else_if_values = (void *)0;

  // FOR LOOP
  ast->for_loop_body = (void *)0;
  ast->for_loop_counter = (void *)0;
  ast->for_loop_range = (void *)0;

  return ast;
}
