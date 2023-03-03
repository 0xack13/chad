#include <getopt.h>
#include <string.h>

#include "../include/manifest/manifest.h"
#include "../include/utils/args.h"
#include "../include/utils/log.h"
#include "../include/utils/utils.h"

chad_args_t PARSE_ARGS(int argc, char **argv) {
  int option;

  static int help_flag = 0;
  static int version_flag = 0;
  int compile_flag = 0;
  int run_flag = 0;

  char *non_opt_arg = NULL;
  char *non_opt_arg2 = NULL;
  char *e_arg = NULL;

  if (argc > 1 && argv[1][0] != '-') {
    non_opt_arg = argv[1];
  }

  if (argc > 2 && argv[2][0] != '-') {
    non_opt_arg2 = argv[2];
  }

  static struct option long_options[] = {
      {"help", no_argument, &help_flag, 1},
      {"version", no_argument, &version_flag, 1},
      {0, 0, 0, 0}};

  while ((option = getopt_long(argc, argv, "e:hv", long_options, NULL)) != -1) {
    switch (option) {
    case 'e':
      e_arg = optarg;
      break;
    case 'h':
      help_flag = 1;
      break;
    case 'v':
      version_flag = 1;
      break;
    case '?':
      log_error(NULL, "unknown option");
    }
  }

  int no_args = 0;
  if (argc == 1) {
    no_args = 1;
  }

  int has_manifest = 0;
  char *filename = NULL;

  has_manifest = file_exists("./chad.chad");

  if (no_args && has_manifest) {
    non_opt_arg = "all";
    // non_opt_arg2 = "run";

    no_args = 0;
  }

  if (non_opt_arg) {
    char *command = non_opt_arg;
    char *command_option = non_opt_arg2;

    if (has_manifest) {
      manifest_t manifest = get_manifest("./chad.chad");

      int contains = manifest_contains(manifest, command);

      if (contains) {
        command_option = read_manifest(manifest, command)->string_value;

        command = "run";
      }
    }

    if (strcmp(command, "run") == 0) {
      filename = command_option;
      run_flag = 1;
    } else if (strcmp(command, "help") == 0) {
      help_flag = 1;
    } else if (strcmp(command, "compile") == 0) {
      filename = command_option;
      compile_flag = 1;
    } else {
      log_error(NULL, "unknown command `%s`", command);
    }
  }

  if (!e_arg) {
    e_arg = "main";
  }

  chad_args_t chad_args = {.filename = filename,
                           .help = help_flag,
                           .version = version_flag,
                           .start = e_arg,
                           .no_args = no_args,
                           .compile = compile_flag,
                           .run = run_flag};

  return chad_args;
}
