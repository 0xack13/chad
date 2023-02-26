#include "../include/parser/parser.h"
#include "../include/utils/log.h"

#include <string.h>

ast_type_t str_to_ast_type(parser_t *parser, char *data_type) {
  if (strcmp(data_type, "bool") == 0) {
    return AST_BOOL;
  } else if (strcmp(data_type, "int") == 0) {
    return AST_INT;
  } else if (strcmp(data_type, "float") == 0) {
    return AST_FLOAT;
  } else if (strcmp(data_type, "char") == 0) {
    return AST_CHAR;
  } else if (strcmp(data_type, "str") == 0) {
    return AST_STRING;
  } else {
    log_error(parser->tracker, "%s is not a type", data_type);
  }
}
