#include "../include/runtime/runtime.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

// #include <string.h>

char *ast_type_to_str(tracker_t *tracker, ast_type_t type) {
  switch (type) {
  case AST_BOOL:
    return "BOOL";
  case AST_INT:
    return "INT";
  case AST_FLOAT:
    return "FLOAT";
  case AST_CHAR:
    return "CHAR";
  case AST_STRING:
    return "STR";
  case AST_NULL:
    return "NULL";
  case AST_FUNCTION_CALL:
    return "FUNCTION CALL";
  case AST_VARIABLE:
    return "VARIABLE";
  case AST_ARITHMETIC_OPERATOR:
    return "ARITHMETIC OPERATOR";
  default: {
    log_error(tracker, "%d is not a type at line %d", type);
  }
  }
}
