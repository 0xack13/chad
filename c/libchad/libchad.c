#include "../include/libchad/libchad.h"
#include "../include/scope/scope.h"
#include "../include/utils/utils.h"

#include <stdio.h>
#include <string.h>

AST_T *core_print(visitor_t *visitor, AST_T *fdef) {

  AST_T *size_ast = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "__vars__size");
  int size = size_ast->variable_definition_value->int_value;

  for (int i = 0; i < size; i++) {
    char arg_name[1024];

    strcpy(arg_name, "__vars__");

    char index_str[1024];
    sprintf(index_str, "%d", i);

    strcat(arg_name, index_str);

    AST_T *vdef = scope_get_variable_definition(
        visitor->tracker, fdef->function_definition_body->scope, arg_name);

    AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

    lib_print(visited_ast);
  }

  return fdef;
}

AST_T *core_println(visitor_t *visitor, AST_T *fdef) {

  AST_T *size_ast = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "__vars__size");
  int size = size_ast->variable_definition_value->int_value;

  for (int i = 0; i < size; i++) {
    char arg_name[1024];

    strcpy(arg_name, "__vars__");

    char index_str[1024];
    sprintf(index_str, "%d", i);

    strcat(arg_name, index_str);

    AST_T *vdef = scope_get_variable_definition(
        visitor->tracker, fdef->function_definition_body->scope, arg_name);

    AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

    lib_print(visited_ast);
  }

  printf("\n");

  return fdef;
}

AST_T *core_exit(visitor_t *visitor, AST_T *fdef) {
  AST_T *vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "exit_code");

  AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

  lib_exit(visited_ast);

  return fdef;
}

AST_T *core_strcat(visitor_t *visitor, AST_T *fdef) {
  AST_T *left_vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "left");

  AST_T *left_ast = visit(visitor, left_vdef->variable_definition_value);

  AST_T *right_vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "right");

  AST_T *right_ast = visit(visitor, right_vdef->variable_definition_value);

  char *returned_str = lib_strcat(left_ast, right_ast);

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  returned_value->string_value = returned_str;

  return returned_value;
}

AST_T *core_pwd(visitor_t *visitor, AST_T *fdef) {

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  returned_value->string_value = lib_pwd();

  return returned_value;
}

AST_T *core_get_timestamp(visitor_t *visitor, AST_T *fdef) {

  AST_T *returned_value = init_ast(visitor->tracker, AST_INT);

  returned_value->int_value = lib_get_timestamp();

  return returned_value;
}

AST_T *core_read_file_to_string(visitor_t *visitor, AST_T *fdef) {
  AST_T *vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "file_path");

  AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  char *string_value = lib_read_file_to_string(visited_ast);

  returned_value->string_value = string_value;

  return returned_value;
}

AST_T *core_http_get(visitor_t *visitor, AST_T *fdef) {
  AST_T *vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "url");

  AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  char *string_value = lib_http_get(visited_ast);

  returned_value->string_value = string_value;

  return returned_value;
}

char *lib_http_get(AST_T *visited_ast) {
  char *url = visited_ast->string_value;

  return http_get(url);
}

AST_T *core_cmd(visitor_t *visitor, AST_T *fdef) {
  AST_T *vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "shell_command");

  AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  lib_cmd(visited_ast);

  return fdef;
}

AST_T *core_chad_run(visitor_t *visitor, AST_T *fdef) {
  AST_T *vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "shell_command");

  AST_T *visited_ast = visit(visitor, vdef->variable_definition_value);

  AST_T *returned_value = init_ast(visitor->tracker, AST_STRING);

  lib_chad_run(visited_ast);

  return fdef;
}

AST_T *core_write_string_to_file(visitor_t *visitor, AST_T *fdef) {
  AST_T *path_vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "file_path");

  AST_T *path_ast = visit(visitor, path_vdef->variable_definition_value);

  AST_T *content_vdef = scope_get_variable_definition(
      visitor->tracker, fdef->function_definition_body->scope, "content");

  AST_T *content_ast = visit(visitor, content_vdef->variable_definition_value);

  lib_write_string_to_file(path_ast, content_ast);

  return fdef;
}

char *get_core_source() {
  char *core_source = "fn println(~){}"
                      "fn print(~){}"
                      "fn exit(exit_code: int){}"
                      "fn read_file_to_string(file_path: str) {}"
                      "fn write_string_to_file(file_path: str, content: str) {}"
                      "fn pwd() {}"
                      "fn get_timestamp(){}"
                      "fn cmd(shell_command: str){}"
                      "fn chad_run(shell_command: str){}"
                      "fn http_get(url: str){}"
                      "fn strcat(left: str, right: str){}";

  char *source = malloc(strlen(core_source) + 1 * sizeof(char));
  strcpy(source, core_source);

  return source;
}

int is_core_function(char *function_name) {
  int found = 0;

  int all_functions_size = 11;
  char *all_functions[11] = {"print",
                             "println",
                             "exit",
                             "read_file_to_string",
                             "write_string_to_file",
                             "pwd",
                             "get_timestamp",
                             "chad_run",
                             "cmd",
                             "http_get",
                             "strcat"};

  for (int i = 0; i < all_functions_size; i++) {
    char *keyword = all_functions[i];

    if (strcmp(keyword, function_name) == 0) {
      found = 1;
    }
  }

  return found;
}
