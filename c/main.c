#include "include/compiler/compiler.h"
#include "include/interpreter/interpreter.h"
#include "include/utils/const.h"
#include "include/utils/utils.h"

#include <string.h>

int main(int argc, char **argv) {

#ifdef PRECOMPILE
  extern char *filename;
  extern char *start_fn;

  chad_args_t chad_args = {.filename = filename,
                           .help = 0,
                           .version = 0,
                           .start = start_fn,
                           .no_args = 0,
                           .compile = 0,
                           .run = 0,
                           .neww = 0,
                           .init = 0,
                           .compile_run = 0};

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

  if (chad_args.neww) {
    log_print("NEW TODO");
    exit(0);
  }

  if (chad_args.init) {
    log_print("INIT TODO");
    exit(0);
  }

  char *file_contents;

  if (chad_args.compile) {
    char *exec_name = compile(chad_args);

    if (chad_args.compile_run) {
      system(exec_name);
    }

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

  interpret(chad_args, file_contents);

  return 0;
}
