#include "../include/manifest/manifest.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

#include <string.h>

manifest_t get_manifest(char *file_path) {

  char *file_source = read_file_to_string(file_path);

  // LEXER
  tracker_t *lexer_tracker = init_tracker();
  lexer_tracker->filename = file_path;
  lexer_tracker->mode = LEXER;

  lexer_t *lexer = init_lexer(file_source);
  lexer->tracker = lexer_tracker;
  bundle_t bundle = get_all_tokens(lexer);
  token_t **all_tokens = bundle.all_tokens;

  // PARSER
  tracker_t *parser_tracker = init_tracker();
  parser_tracker->filename = file_path;
  parser_tracker->mode = PARSER;

  parser_t *parser = init_parser(all_tokens, bundle.tokens_size);

  parser->tracker = parser_tracker;
  parser->tracker->line = 1;
  parser->tracker->column = 1;

  scope_t *scope = init_scope();
  AST_T *ast_root = parser_start(parser, scope);

  tracker_t *visitor_tracker = init_tracker();
  visitor_tracker->filename = file_path;
  visitor_t *visitor = init_visitor();
  visitor->tracker = visitor_tracker;
  visitor->tracker->mode = RUNTIME;
  visit(visitor, ast_root);

  manifest_t manifest = {.root_node = ast_root, .visitor = visitor};

  return manifest;
}

int manifest_contains(manifest_t manifest, char *command) {
  int contains = 0;

  contains = variable_is_defined(manifest.root_node->scope, command);

  return contains;
}

AST_T *read_manifest(manifest_t manifest, char *command) {

  AST_T *vardef = scope_get_variable_definition(
      manifest.root_node->scope->tracker, manifest.root_node->scope, command);

  return vardef->variable_definition_value;
}