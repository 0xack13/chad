#ifndef AST_H
#define AST_H
#include <stdbool.h>
#include <stdlib.h>

#include "../utils/log.h"

typedef enum { O_NONE, O_ADD, O_SUB, O_DIV, O_MUL } operator_t;

typedef enum {
  BLOCK_NONE,
  BLOCK_IF,
  BLOCK_FOR,
  BLOCK_LOOP,
  BLOCK_FN
} block_type_t;

typedef enum {
  CON_NONE,
  CON_GT,
  CON_LT,
  CON_EQ,
  CON_NEQ,
  CON_GTE,
  CON_LTE,
} conditional_operator_t;

typedef enum { LO_NONE, LO_NOT, LO_AND, LO_OR } logical_operator_t;

typedef enum {
  AST_VARIABLE_DEFINITION,
  AST_VARIABLE_REDEFINITION,
  AST_FUNCTION_DEFINITION,
  AST_RETURN_STATEMENT,
  AST_BREAK_STATEMENT,
  AST_USE_STATEMENT,
  AST_CONTINUE_STATEMENT,
  AST_VARIABLE,
  AST_FUNCTION_CALL,
  AST_CHAR,
  AST_STRING,
  AST_INT,
  AST_FLOAT,
  AST_BOOL,
  AST_NUMBER_RANGE,
  AST_ARITHMETIC_OPERATOR,
  AST_CONDITIONAL_OPERATOR,
  AST_EXPRESSION,
  AST_CONDITIONAL_EXPRESSION,
  AST_LOGICAL_OPERATOR,
  AST_BLOCK,
  AST_IF_STATEMENT,
  AST_FOR_LOOP,
  AST_LOOP_LOOP,
  AST_EOF,
  AST_NULL
} ast_type_t;

typedef struct AST_STRUCT {
  // META
  int line;
  int column;
  
  // TYPE
  ast_type_t type;

  // SCOPE
  struct SCOPE_STRUCT *scope;

  // PARENT
  struct AST_STRUCT *parent_block;

  // VARIABLE DEFINITION
  char *variable_definition_variable_name;
  struct AST_STRUCT *variable_definition_value;
  ast_type_t variable_definition_type;

  // VARIABLE REDEFINITION
  char *variable_redefinition_variable_name;
  struct AST_STRUCT *variable_redefinition_value;
  ast_type_t variable_redefinition_type;

  // FUNCTION_DEFINITION
  struct AST_STRUCT *function_definition_body;
  char *function_definition_name;
  struct AST_STRUCT **function_definition_args;
  long function_definition_args_size;
  int function_definition_is_variadic;

  // VARIABLE
  char *variable_name;

  // FUNCTION_CALL
  char *function_call_name;
  struct AST_STRUCT **function_call_arguments;
  long function_call_arguments_size;

  // ACTUAL VALUES
  char *string_value;
  char char_value;
  int int_value;
  bool bool_value;
  float float_value;

  // NUMBER RANGE
  struct AST_STRUCT *number_range_start;
  struct AST_STRUCT *number_range_end;

  // ARITHMETIC OPERATOR
  operator_t operator_value;

  // CONDITIONAL OPERATOR
  conditional_operator_t conditional_operator_value;

  // LOGICAL OPERATOR
  logical_operator_t logical_operator_value;

  // BLOCK
  struct AST_STRUCT **block_values;
  long block_size;
  int current_block_item;
  block_type_t block_type;
  int main_block;

  // EXPRESSION
  struct AST_STRUCT **expression_values;
  long expression_size;

  // CONDITIONAL EXPRESSION
  struct AST_STRUCT **conditional_expression_values;
  long conditional_expression_size;

  // IF STATEMENT
  struct AST_STRUCT *if_statement_condition;
  struct AST_STRUCT *if_statement_body;
  int has_else;
  struct AST_STRUCT *else_body;
  long else_if_count;
  struct AST_STRUCT **else_if_values;

  // FOR LOOP
  struct AST_STRUCT *for_loop_counter;
  struct AST_STRUCT *for_loop_body;
  struct AST_STRUCT *for_loop_range;

  // LOOP LOOP
  struct AST_STRUCT *loop_loop_body;

  // RETURN STATEMENT
  struct AST_STRUCT *returned_value;

  // USE STATEMENT
  char *use_file_path;
  char *use_function_name;

} AST_T;

AST_T *init_ast(tracker_t *tracker, int type);

#endif
