#ifndef CHAD_ARGS_H
#define CHAD_ARGS_H

typedef struct CHAD_ARGS_STRUCT {
  char *filename;
  char* package_name;
  char *start;
  int help;
  int version;
  int no_args;
  int compile;
  int run;
  int neww;
  int init;
  int compile_run;
} chad_args_t;

chad_args_t PARSE_ARGS(int argc, char **argv);

#endif
