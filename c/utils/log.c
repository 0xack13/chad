#include "../include/utils/log.h"

typedef enum {
  BLACK,
  GREEN,
  YELLOW,
  MAGENTA,
  CYAN,
  WHITE,
  BLUE,
  RED,
  DEFAULT
} color_t;

typedef enum { BOLD, UNDERLINE } style_t;

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

tracker_t *init_tracker() {
  tracker_t *tracker = calloc(1, sizeof(struct TRACKER_STRUCT));
  tracker->column = 1;
  tracker->filename = (void *)0;
  tracker->line = 1;
  tracker->mode = MODE_NONE;

  return tracker;
}

void set_style(style_t style) {
  switch (style) {
  case BOLD: {
    printf("\033[1m");
    break;
  }

  case UNDERLINE: {
    printf("\033[4m");
    break;
  }
  }
}

void set_color(color_t color) {
  switch (color) {

  case BLACK:
    printf("\033[30m");
    break;

  case GREEN:
    printf("\033[32m");
    break;

  case YELLOW:
    printf("\033[33m");
    break;

  case MAGENTA:
    printf("\033[35m");
    break;

  case CYAN:
    printf("\033[36m");
    break;

  case WHITE:
    printf("\033[37m");
    break;

  case RED:
    printf("\033[31m");
    break;
  case BLUE:
    printf("\033[34m");
    break;

  case DEFAULT:
    printf("\033[0m");
    break;
  }
}

void clear_view() {
  printf("\033c"); // clear the terminal
}

void reset_color() { set_color(DEFAULT); }

// BUG: passing an int to a %s format string causes segfault
void log_print(const char *format, ...) {
  set_color(GREEN);
  printf("log: ");
  reset_color();

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}


void log_warning(tracker_t *tracker, char *format, ...) {
  printf("\n");
  if (tracker) {
    int print_mode = 0;

    if (print_mode) {
      switch (tracker->mode) {
      case LEXER: {
        printf("LEXER - ");
        break;
      }

      case PARSER: {
        printf("PARSER - ");
        break;
      }

      case RUNTIME: {
        printf("RUNTIME - ");
        break;
      }
      case MODE_NONE: {
        printf("NONE MODE!!");
        exit(1);
      }
      }
    }

    set_style(BOLD);
    printf("%s %d:%d ", tracker->filename, tracker->line, tracker->column);
  }

  set_color(YELLOW);
  printf("warning: ");
  reset_color();

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}

void log_error(tracker_t *tracker, char *format, ...) {
  printf("\n");
  if (tracker) {
    int print_mode = 1;

    if (print_mode) {
      switch (tracker->mode) {
      case LEXER: {
        printf("LEXER - ");
        break;
      }

      case PARSER: {
        printf("PARSER - ");
        break;
      }

      case RUNTIME: {
        printf("RUNTIME - ");
        break;
      }
      case MODE_NONE: {
        printf("NONE MODE!!");
        exit(1);
      }
      }
    }

    set_style(BOLD);
    printf("%s %d:%d ", tracker->filename, tracker->line, tracker->column);
  }

  set_color(RED);
  printf("error: ");
  reset_color();

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
  exit(1);
}
