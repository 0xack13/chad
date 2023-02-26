#ifndef LIBCHAD_H
#define LIBCHAD_H

#include "../runtime/runtime.h"

AST_T *core_print(visitor_t *visitor, AST_T *fdef);

AST_T *core_println(visitor_t *visitor, AST_T *fdef);

AST_T *core_exit(visitor_t *visitor, AST_T *fdef);

AST_T *core_read_file_to_string(visitor_t *visitor, AST_T *fdef);

AST_T *core_write_string_to_file(visitor_t *visitor, AST_T *fdef);

AST_T *core_pwd(visitor_t *visitor, AST_T *fdef);

AST_T *core_get_timestamp(visitor_t *visitor, AST_T *fdef);

AST_T *core_cmd(visitor_t *visitor, AST_T *fdef);

AST_T *core_chad_run(visitor_t *visitor, AST_T *fdef);

AST_T *core_http_get(visitor_t *visitor, AST_T *fdef);

AST_T *core_strcat(visitor_t *visitor, AST_T *fdef);

extern void lib_print(AST_T *visited_ast);

extern void lib_exit(AST_T *visited_ast);

extern char *lib_read_file_to_string(AST_T *visited_ast);

char *lib_http_get(AST_T *visited_ast);

extern char *lib_cmd(AST_T *visited_ast);

extern char *lib_chad_run(AST_T *visited_ast);

extern char *lib_write_string_to_file(AST_T *path_ast, AST_T *content_path);

extern char *lib_pwd();

extern char *lib_strcat(AST_T *left_ast, AST_T *right_ast);

extern int lib_get_timestamp();

char *get_core_source();

#endif