#ifndef CONST_H
#define CONST_H

#define HELP                                                                   \
  "\
\n\
 CCCC  H   H   A   DDDD \n\
C      H   H  A A  D   D \n\
C      HHHHH AAAAA D   D \n\
C      H   H A   A D   D \n\
 CCCC  H   H A   A DDDD \n\
\n\
Chad Programming Language\n\
\n\
USAGE:\n\
  chad [COMMAND] [OPTIONS]...\n\
COMMANDS:\n\
  run            run a script.\n\
  help           show this screen.\n\
  new            create a new chad package\n\
  init           initialize a new chad package\n\
  compile        compile a script into a binary executable\n\
  compile-run    compile and run a script as a binary executable\n\
OPTIONS:\n\
  -h --help      Show this screen.\n\
  -v --version   Show installed version.\n\
  -e             set the entry function. default: main \n\
     --debug     run in debug mode\n\
EXAMPLES:\n\
  chad run ./hello-world.chad\n\
  chad run https://hiro.codes/files/hello.chad\n\
  chad run ./hello-chad.chad -e my_function\n\
  \n\
  chad compile ./hello-world.chad\n\
  chad new my_package\n\
\n"

#define VERSION "Chad 0.11.2"

#endif