#ifndef LOG_H
#define LOG_H

typedef enum {
  LEXER,
  PARSER,
  RUNTIME,
  MODE_NONE,
} tracker_mode_t;

typedef struct TRACKER_STRUCT {
  int line;
  int column;
  char *filename;
  tracker_mode_t mode;
} tracker_t;

tracker_t *init_tracker();

void log_print(const char *format, ...);

void log_warning(tracker_t *tracker, char *format, ...);

void log_error(tracker_t *tracker, char *format, ...);

#endif