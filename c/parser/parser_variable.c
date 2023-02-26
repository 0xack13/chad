#include "../include/utils/log.h"
#include "../include/utils/utils.h"

AST_T *parse_variable_redefinition(parser_t *parser, scope_t *scope) {
  char *variable_name = parser->prev_token->value;

  parser_expect(parser, T_EQUALS); // expect equals sign
  AST_T *variable_value = parse_term(parser, scope);

  AST_T *variable_definition =
      init_ast(parser->tracker, AST_VARIABLE_REDEFINITION);
  variable_definition->variable_redefinition_variable_name = variable_name;
  variable_definition->variable_redefinition_value = variable_value;

  variable_definition->scope = scope;
  variable_definition->variable_redefinition_type =
      variable_definition->variable_redefinition_value->type; // infer type

  return variable_definition;
}

AST_T *parse_variable_definition(parser_t *parser, scope_t *scope) {
  parser_expect(parser, T_ID); // expect the let identifier
  char *variable_definition_variable_name = parser->current_token->value;

  parser_expect(parser, T_ID);                        // variable name
  int passed = parser_expect_check(parser, T_EQUALS); // check for equal sign

  if (passed) {                      // type was not specified
    parser_expect(parser, T_EQUALS); // expect equals sign
    AST_T *variable_definition_value = parse_term(parser, scope);

    AST_T *variable_definition =
        init_ast(parser->tracker, AST_VARIABLE_DEFINITION);
    variable_definition->variable_definition_variable_name =
        variable_definition_variable_name;
    variable_definition->variable_definition_value = variable_definition_value;

    variable_definition->scope = scope;
    variable_definition->variable_definition_type =
        variable_definition->variable_definition_value->type; // infer type

    return variable_definition;
  } else {                          // type was specified
    parser_expect(parser, T_COLON); // expect colon sign

    // save the data type that was specified
    char *data_type = parser->current_token->value;

    parser_expect(parser, T_ID); // expect data type

    ast_type_t formal_type = str_to_ast_type(parser, data_type);

    parser_expect(parser, T_EQUALS); // expect equal sign

    AST_T *variable_definition =
        init_ast(parser->tracker, AST_VARIABLE_DEFINITION);
    variable_definition->scope = scope;

    AST_T *variable_definition_value = parse_term(
        parser, scope); // parse the expression that was provided as the value

    variable_definition->variable_definition_variable_name =
        variable_definition_variable_name;

    variable_definition->variable_definition_value = variable_definition_value;

    variable_definition->variable_definition_type = formal_type;

    // compare the data type of the gotten expression with the one that was
    // specified. panic if they are different
    // check the data types directly if the assignee is not an identifier
    if (variable_definition->variable_definition_value->type !=
            AST_FUNCTION_CALL &&
        parser->prev_token->type != T_ID &&
        variable_definition->variable_definition_type !=
            variable_definition->variable_definition_value->type) {
      log_error(parser->tracker,
                "variable `%s` with type %s cannot be assigned to "
                "`%s` with type %s",
                variable_definition_variable_name,
                token_type_to_string(str_to_token_type(parser, data_type)),
                parser->prev_token->value,
                token_type_to_string(parser->prev_token->type));
    }

    return variable_definition;
  }
}

AST_T *parse_variable(parser_t *parser, scope_t *scope) {
  char *token_value = parser->prev_token->value;

  if (is_keyword(token_value)) {
    log_error(parser->tracker, "cannot use keyword `%s` as variable name",
              token_value);
  }

  AST_T *ast_variable = init_ast(parser->tracker, AST_VARIABLE);
  ast_variable->variable_name = token_value;

  ast_variable->scope = scope;

  return ast_variable;
}
