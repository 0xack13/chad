#ifndef VISITOR_H
#define VISITOR_H
#include "../ast/ast.h"
#include "../utils/log.h"

typedef struct VISITOR_STRUCT {
  // VISITOR FLAGS
  tracker_t *tracker;

  int returned;
  int loop_break;
  int loop_continue;

} visitor_t;

void add_call_stack(visitor_t *visitor, AST_T *node);

void pop_call_stack(visitor_t *visitor);

visitor_t *init_visitor();

AST_T *visit(visitor_t *visitor, AST_T *node);

AST_T *visit_for_loop(visitor_t *visitor, AST_T *node);

AST_T *visit_loop_loop(visitor_t *visitor, AST_T *node);

AST_T *visit_if_statement(visitor_t *visitor, AST_T *node);

AST_T *visit_variable_definition(visitor_t *visitor, AST_T *node);

AST_T *visit_function_definition(visitor_t *visitor, AST_T *node);

int is_core_function(char *function_name);

AST_T *visit_core_function(visitor_t *visitor, AST_T *fdef,
                           char *function_name);

AST_T *visit_variable_redefinition(visitor_t *visitor, AST_T *node);

AST_T *visit_variable(visitor_t *visitor, AST_T *node);

AST_T *visit_arithmetic_expression(visitor_t *visitor, AST_T *node);

AST_T *visit_conditional_expression(visitor_t *visitor, AST_T *node);

AST_T *visit_function_call(visitor_t *visitor, AST_T *node);

AST_T *visit_return_statement(visitor_t *visitor, AST_T *node);

AST_T *visit_break_statement(visitor_t *visitor, AST_T *node);

AST_T *visit_continue_statement(visitor_t *visitor, AST_T *node);

AST_T *visit_use_statement(visitor_t *visitor, AST_T *node);

AST_T *visit_string(visitor_t *visitor, AST_T *node);

AST_T *visit_int(visitor_t *visitor, AST_T *node);

AST_T *visit_operator(visitor_t *visitor, AST_T *node);

AST_T *visit_conditional_operator(visitor_t *visitor, AST_T *node);

AST_T *visit_logical_operator(visitor_t *visitor, AST_T *node);

AST_T *visit_char(visitor_t *visitor, AST_T *node);

AST_T *visit_float(visitor_t *visitor, AST_T *node);

AST_T *visit_bool(visitor_t *visitor, AST_T *node);

AST_T *visit_block(visitor_t *visitor, AST_T *node);

bool apply_logical_operator(bool result_buffer, bool buffer,
                            logical_operator_t current_logical_operator);
#endif
