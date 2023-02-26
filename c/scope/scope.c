#include "../include/scope/scope.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <stdio.h>
#include <string.h>

scope_t *init_scope() {
  scope_t *scope = calloc(1, sizeof(struct SCOPE_STRUCT));

  scope->function_definitions = (void *)0;
  scope->function_definitions_size = 0;

  scope->variable_definitions = (void *)0;
  scope->variable_definitions_size = 0;

  scope->return_type = AST_NULL;
  scope->returned_value = (void *)0;

  scope->loop_break = 0;
  scope->loop_continue = 0;

  scope->returned = 0;

  scope->parent_scope = (void *)0;

  scope->tracker = (void *)0;

  return scope;
}

void clear_scope(scope_t *scope) {

  scope->variable_definitions_size = 0;
  scope->variable_definitions = (void *)0;
  scope->returned_value = (void *)0;
}

void scope_set_return_type(scope_t *scope, ast_type_t *type) {}

void scope_set_return_value(scope_t *scope, AST_T *value) {}

int variable_is_defined(scope_t *scope, char *variable_name) {
  int defined = 0;

  for (int i = 0; i < scope->variable_definitions_size; i++) {
    AST_T *vdef = scope->variable_definitions[i];

    if (strcmp(vdef->variable_definition_variable_name, variable_name) == 0) {
      defined = 1;
    }
  }

  return defined;
}

int function_is_defined(scope_t *scope, char *function_name) {
  int defined = 0;

  for (int i = 0; i < scope->function_definitions_size; i++) {
    AST_T *fdef = scope->function_definitions[i];

    if (strcmp(fdef->function_definition_name, function_name) == 0) {
      defined = 1;
    }
  }

  return defined;
}

int is_keyword(char *name) {
  int found = 0;

  int all_keywords_size = 18;
  char *all_keywords[18] = {"let",    "fn",    "if",       "for",  "loop",
                            "return", "break", "continue", "else", "true",
                            "false",  "in",    "use",      "bool", "int",
                            "float",  "str",   "char"};

  // reserved keywords
  int all_reserved_size = 61;
  char *all_reserved[61] = {
      "struct",   "auto",    "default", "double",   "extern",   "long",
      "register", "short",   "union",   "unsigned", "volatile", "enum",
      "string",   "define",  "const",   "var",      "static",   "is",
      "goto",     "switch",  "match",   "case",     "do",       "while",
      "until",    "private", "this",    "pub",      "public",   "new",
      "throw",    "try",     "catch",   "import",   "export",   "as",
      "with",     "module",  "package", "super",    "global",   "local",
      "Fn",       "call",    "spawn",   "thread",   "go",       "sync",
      "async",    "await",   "yield",   "typeof",   "debug",    "eval",
      "default",  "void",    "null",    "nil",      "NULL",     "typedef",
      "type"};

  for (int i = 0; i < all_keywords_size; i++) {
    char *keyword = all_keywords[i];

    if (strcmp(keyword, name) == 0) {
      found = 1;
    }
  }

  // reserved keywords
  for (int i = 0; i < all_reserved_size; i++) {
    char *reserved = all_reserved[i];

    if (strcmp(reserved, name) == 0) {
      found = 1;
    }
  }

  return found;
}

AST_T *scope_add_function_definition(tracker_t *tracker, scope_t *scope,
                                     AST_T *fdef) {
  if (function_is_defined(scope, fdef->function_definition_name)) {
    log_error(tracker, "function `%s` is already defined",
              fdef->function_definition_name);
  }

  if (is_keyword(fdef->function_definition_name)) {
    log_error(tracker, "cannot use keyword `%s` as function name",
              fdef->function_definition_name);
  }

  scope->function_definitions_size += 1;

  if (scope->function_definitions == (void *)0) {
    scope->function_definitions = calloc(1, sizeof(struct AST_STRUCT *));
  } else {
    scope->function_definitions =
        realloc(scope->function_definitions, scope->function_definitions_size *
                                                 sizeof(struct AST_STRUCT **));
  }

  scope->function_definitions[scope->function_definitions_size - 1] = fdef;

  return fdef;
}

AST_T *scope_get_function_definition(tracker_t *tracker, scope_t *scope,
                                     const char *fname) {
  // for local scope
  for (int i = 0; i < scope->function_definitions_size; i++) {
    AST_T *fdef = scope->function_definitions[i];

    if (strcmp(fdef->function_definition_name, fname) == 0) {
      return fdef;
    }
  }

  // for global scope
  scope_t *current_scope;
  current_scope = scope->parent_scope;

  while (current_scope != NULL) {
    for (int i = 0; i < current_scope->function_definitions_size; i++) {
      AST_T *fdef = current_scope->function_definitions[i];

      if (strcmp(fdef->function_definition_name, fname) == 0) {
        return fdef;
      }
    }

    current_scope = current_scope->parent_scope;
  }

  log_error(tracker, "could not get function `%s` definition in scope", fname);
}

AST_T *scope_add_variable_definition(tracker_t *tracker, scope_t *scope,
                                     AST_T *vdef) {
  if (variable_is_defined(scope, vdef->variable_definition_variable_name)) {
    log_error(tracker, "variable `%s` is already defined",
              vdef->variable_definition_variable_name);
  }

  if (is_keyword(vdef->variable_definition_variable_name)) {
    log_error(tracker, "cannot use keyword `%s` as variable name",
              vdef->variable_definition_variable_name);
  }

  if (scope->variable_definitions == (void *)0) {
    scope->variable_definitions = calloc(1, sizeof(struct AST_STRUCT *));
    scope->variable_definitions[0] = vdef;
    scope->variable_definitions_size += 1;
  } else {
    scope->variable_definitions_size += 1;
    scope->variable_definitions =
        realloc(scope->variable_definitions,
                scope->variable_definitions_size * sizeof(struct AST_STRUCT *));
    scope->variable_definitions[scope->variable_definitions_size - 1] = vdef;
  }

  return vdef;
}

AST_T *scope_get_variable_definition(tracker_t *tracker, scope_t *scope,
                                     const char *name) {
  // for local scope
  for (int i = 0; i < scope->variable_definitions_size; i++) {
    AST_T *vdef = scope->variable_definitions[i];

    if (strcmp(vdef->variable_definition_variable_name, name) == 0) {
      return vdef;
    }
  }

  // for global scope

  scope_t *current_scope;
  current_scope = scope->parent_scope;

  while (current_scope != NULL) {
    for (int i = 0; i < current_scope->variable_definitions_size; i++) {
      AST_T *vdef = current_scope->variable_definitions[i];

      if (strcmp(vdef->variable_definition_variable_name, name) == 0) {
        return vdef;
      }
    }

    current_scope = current_scope->parent_scope;
  }

  log_error(tracker, "could not get variable `%s` definition in scope", name);
}
