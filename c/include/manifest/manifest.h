#ifndef MANIFEST_H
#define MANIFEST_H

#include "../ast/ast.h"
#include "../runtime/runtime.h"


typedef struct MANIFEST_STRUCT {
  AST_T *root_node;
  visitor_t *visitor;
} manifest_t;

manifest_t get_manifest(char *file_path);

int manifest_contains(manifest_t manifest, char *command);

AST_T *read_manifest(manifest_t manifest, char *command);

#endif