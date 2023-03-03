#ifndef PRECOMPILE
#include "../../target/chad_precompiled.h"
#include "../include/utils/args.h"
#include "../include/utils/utils.h"

#include <stdio.h>

void compile(chad_args_t chad_args) {
  log_print("Compiling `%s`", chad_args.filename);

  char *file_contents = read_file_to_string(chad_args.filename);

  system("mkdir -p tmp");

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

  system("cd tmp && gcc -c tmp_src.c");

  char command[1024];

  char *exec_name = extract_filename(chad_args.filename);
  log_print("creating executable `%s`", exec_name);

  sprintf(command, "gcc -Os ./tmp/*.o -o %s -L./tmp/ -llibchad -lcurl",
          exec_name);
  system(command);

  sprintf(command, "chmod +x %s", exec_name);
  system(command);

  sprintf(command, "strip %s", exec_name);
  system(command);

  system("rm -r tmp");

  log_print("Finished!");
  exit(0);
}

#endif
