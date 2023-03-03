#ifndef UTILS_H
#define UTILS_H

#include "../parser/parser.h"
#include "../runtime/runtime.h"

#define SPACE_CHAR ' '
#define NEWLINE_CHAR 10
#define NULL_CHAR '\0'
#define DOUBLE_QUOTE_CHAR '"'
#define SINGLE_QUOTE_CHAR '\''
#define UNDERSCORE_CHAR '_'
#define COLON_CHAR ':'
#define DOT_CHAR '.'
#define SEMI_CHAR ';'
#define TAB_CHAR '\t'
#define LBRACKET_CHAR '['
#define RBRACKET_CHAR ']'
#define RANGLE_CHAR '>'
#define LANGLE_CHAR '<'
#define LBRACE_CHAR '{'
#define RBRACE_CHAR '}'
#define PERCENT_CHAR '%'
#define MINUS_CHAR '-'
#define PLUS_CHAR '+'
#define ASTERIX_CHAR '*'
#define EQUAL_CHAR '='
#define EXCLAMATION_CHAR '!'
#define AMPERSAND_CHAR '&'
#define VBAR_CHAR '|'
#define COMMA_CHAR ','
#define DOLLAR_CHAR '$'
#define LPAREN_CHAR '('
#define RPAREN_CHAR ')'
#define FSLASH_CHAR '/'
#define BSLASH_CHAR '\\'
#define TILDE_CHAR '~'

typedef enum {
  DECIMAL,
  FLOAT,
  OCTAL,
  HEXADECIMAL,
  BINARY,
  INVALID
} NumberFormat;

char *str_replace(char *str, const char *old, const char *neww);

char *replace_newlines_with_escapes(char *str);

char *remove_extension(char *filename);

char *extract_filename(const char *path);

void panic(char *message);

char *read_file_to_string(char *filename);

int file_exists(char *filename);

int is_url(char *path);

char *download_file_to_string(char *url);

char *run_shell_command(char *command);

char *http_get(char *url);

void change_working_dir(char *dirname);

int file_exists(char *filename);

char *get_working_dir();

char *extract_working_dir(char *file_path);

int file_has_extension(const char *file_name, const char *extension);

void print_all_tokens(token_t **all_tokens);

void print_all_nodes(AST_T *parser);

NumberFormat check_number(const char *str);

ast_type_t str_to_ast_type(parser_t *parser, char *data_type);

token_type_t str_to_token_type(parser_t *parser, char *data_type);

char *ast_type_to_str(tracker_t *tracker, ast_type_t type);

char *token_type_to_string(token_type_t type);

char *token_type_to_str(parser_t *parser, token_type_t type);

bundle_t get_all_tokens(lexer_t *lexer);

#endif