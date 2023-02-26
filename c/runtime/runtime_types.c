#include "../include/libchad/libchad.h"

AST_T *visit_string(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_int(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_char(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_float(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_operator(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_conditional_operator(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_logical_operator(visitor_t *visitor, AST_T *node) { return node; }

AST_T *visit_bool(visitor_t *visitor, AST_T *node) { return node; }
