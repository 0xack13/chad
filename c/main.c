#include "include/runtime/runtime.h"
#include "include/utils/args.h"
#include "include/utils/const.h"
#include "include/utils/log.h"
#include "include/utils/utils.h"

#ifndef PRECOMPILE
#include "../target/chad_precompiled.h"
#endif

#include <stdio.h>
#include <string.h>

char *str_replace(char *str, const char *old, const char *new) {
  char *result;
  int i, count = 0;
  size_t newlen = strlen(new);
  size_t oldlen = strlen(old);

  for (i = 0; str[i] != '\0';) {
    if (strstr(&str[i], old) == &str[i]) {
      count++;
      i += oldlen;
    } else {
      i++;
    }
  }

  result = (char *)malloc(i + count * (newlen - oldlen) + 1);

  i = 0;
  while (*str) {
    if (strstr(str, old) == str) {
      strcpy(&result[i], new);
      i += newlen;
      str += oldlen;
    } else {
      result[i++] = *str++;
    }
  }

  result[i] = '\0';
  return result;
}

char *replace_newlines_with_escapes(char *str) {
  size_t len = strlen(str);
  char *escaped_str = (char *)malloc(
      len * 2 + 1); // allocate enough memory for worst-case scenario
  size_t j = 0;

  for (size_t i = 0; i < len; i++) {
    if (str[i] == '\n') {
      escaped_str[j++] = '\\';
      escaped_str[j++] = 'n';
    } else {
      escaped_str[j++] = str[i];
    }
  }

  escaped_str[j] = '\0'; // null-terminate the string
  return escaped_str;
}

int main(int argc, char **argv) {

#ifdef PRECOMPILE
  extern char *filename;
  extern char *start_fn;

  chad_args_t chad_args = {.filename = filename,
                           .help = 0,
                           .version = 0,
                           .start = start_fn,
                           .no_args = 0,
                           .compile = 0};

  int remote = 0;

  extern char *file_contents;
#endif

#ifndef PRECOMPILE

  chad_args_t chad_args = PARSE_ARGS(argc, argv);

  if (chad_args.help) {
    log_print(HELP);
    exit(0);
  }
  if (chad_args.version) {
    log_print(VERSION);
    exit(0);
  }
  if (chad_args.no_args) {
    log_print(HELP);
    exit(0);
  }

  char *file_contents;

  if (chad_args.compile) {
    log_print("Compiling %s", chad_args.filename);

    file_contents = read_file_to_string(chad_args.filename);

    system("mkdir -p tmp && cd tmp && rm ./*.o ./*.a ./*.c");

    FILE *out_file = fopen("./tmp/chad_precompiled.a", "wb");
    fwrite(__target_chad_precompiled_a, sizeof(__target_chad_precompiled_a), 1,
           out_file);
    fclose(out_file);

    system("cd tmp && ar x ./chad_precompiled.a");

    system("cd tmp && ar x ./precompiled.a");

    system("cd tmp && touch tmp_src.c");

    //
    FILE *tmp_file = fopen("./tmp/tmp_src.c", "a");

    fprintf(tmp_file, "char* start_fn = \"%s\"; \n", chad_args.start);

    fprintf(tmp_file, "char* filename = \"%s\"; \n", chad_args.filename);

    char *contents = str_replace(file_contents, "\"", "\\\"");
    char *escaped_contents = replace_newlines_with_escapes(contents);
    fprintf(tmp_file, "char *file_contents = \"%s\"; \n", escaped_contents);

    fclose(tmp_file);
    // char tmp_buff[256];

    // sprintf(tmp_buff, "%s", );

    // system();

    system("cd tmp && gcc -c tmp_src.c");

    // FIXME: linking fails because of rust library
    system("ld ./tmp/*.o -o final -lc -lcurl -L./tmp/ -llibchad -lc "
           "-L/usr/lib/gcc/x86_64-linux-gnu/12/ -lgcc_s");

    system("chmod +x final");

    log_print("Finished!");
    exit(0);
  }

  int remote = 0;

  if (is_url(chad_args.filename)) {
    remote = 1;
    file_contents = download_file_to_string(chad_args.filename);
  } else {
    file_contents = read_file_to_string(chad_args.filename);
  }

#endif

  if (!file_has_extension(chad_args.filename, ".chad") &&
      !file_has_extension(chad_args.filename, ".chd")) {
    log_warning(NULL, "%s does not have a .chad or .chd extension",
                chad_args.filename);
  }

  if (strlen(file_contents) < 2) {
    log_error(NULL, "%s is empty", chad_args.filename);
  }

  char *working_dir = extract_working_dir(chad_args.filename);
  if (!remote && strlen(working_dir) > 0) {
    change_working_dir(working_dir);
  } else {
    change_working_dir("./");
  }

  // LEXER
  tracker_t *lexer_tracker = init_tracker();
  lexer_tracker->filename = chad_args.filename;
  lexer_t *lexer = init_lexer(file_contents);
  lexer->tracker = lexer_tracker;
  lexer->tracker->mode = LEXER;
  bundle_t bundle = get_all_tokens(lexer);
  token_t **all_tokens = bundle.all_tokens;
  // print_all_tokens(all_tokens);

  // PARSER
  tracker_t *parser_tracker = init_tracker();
  parser_tracker->filename = chad_args.filename;
  parser_t *parser = init_parser(all_tokens, bundle.tokens_size);
  parser->tracker = parser_tracker;
  parser->tracker->mode = PARSER;
  parser->tracker->line = 1;
  parser->tracker->column = 1;
  scope_t *start_scope = init_scope();
  start_scope->tracker = parser->tracker;
  AST_T *root = parser_start(parser, start_scope);

  // construct a use statement for the core library
  AST_T *core_lib = init_ast(parser->tracker, AST_USE_STATEMENT);
  char *core_path = "core";
  char *core_name = "*";

  core_lib->use_file_path = malloc((strlen(core_path) + 1) * sizeof(char));
  core_lib->use_function_name = malloc((strlen(core_name) + 1) * sizeof(char));
  strcpy(core_lib->use_file_path, core_path);
  strcpy(core_lib->use_function_name, core_name);

  // add a use statement for the core library
  add_node_to_tail(core_lib, root);

  root = parser_hoist(parser, root, AST_USE_STATEMENT);
  root = parser_import(parser, root);
  root = parser_hoist(parser, root, AST_FUNCTION_DEFINITION);

  // construct a main function call
  AST_T *main_call = init_ast(parser->tracker, AST_FUNCTION_CALL);
  main_call->scope = root->scope;
  main_call->function_call_name = chad_args.start;
  main_call->function_call_arguments_size = 0;

  // add the main function call as the first node
  add_node_to_tail(main_call, root);

  // add an EOF node
  add_node_to_tail(init_ast(parser->tracker, AST_EOF), root);

  // print_all_nodes(root);

  // RUNTIME
  // BUG: i get a segfault on recurssive functions possibly due to stack
  // overflow
  tracker_t *visitor_tracker = init_tracker();
  visitor_tracker->filename = chad_args.filename;
  visitor_t *visitor = init_visitor();
  visitor->tracker = visitor_tracker;
  visitor->tracker->mode = RUNTIME;
  visit(visitor, root);

  log_error(NULL, "reached end of main file");

  return 0;
}
