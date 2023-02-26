#ifndef SCOPE_H
#define SCOPE_H
#include "../utils/log.h"
#include "../ast/ast.h"

typedef struct SCOPE_STRUCT {
  AST_T **function_definitions;
  int function_definitions_size;

  AST_T **variable_definitions;
  int variable_definitions_size;

  ast_type_t return_type;
  struct AST_STRUCT *returned_value;

  int loop_break;
  int loop_continue;

  int returned;

  struct SCOPE_STRUCT *parent_scope;

  tracker_t *tracker;

} scope_t;

scope_t *init_scope();

void clear_scope(scope_t *scope);

AST_T *scope_add_function_definition(tracker_t *tracker, scope_t *scope,
                                     AST_T *fdef);

AST_T *scope_get_function_definition(tracker_t *tracker, scope_t *scope,
                                     const char *fname);

AST_T *scope_add_variable_definition(tracker_t *tracker, scope_t *scope,
                                     AST_T *vdef);

AST_T *scope_redefine_variable(tracker_t *tracker, scope_t *scope, AST_T *vdef);

AST_T *scope_get_variable_definition(tracker_t *tracker, scope_t *scope,
                                     const char *name);

void scope_set_return_type(scope_t *scope, ast_type_t *type);

ast_type_t *scope_get_return_type(scope_t *scope);

AST_T *scope_get_return_value(scope_t *scope);

void scope_set_return_value(scope_t *scope, AST_T *value);

int is_keyword(char *name);

int variable_is_defined(scope_t *scope, char *variable_name);

#endif
