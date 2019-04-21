//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TCODE_H
#define TLANG_TCODE_H

#include "tinterpreter.h"

ArgumentList* crb_create_argument_list(Expression *expression);
ArgumentList* crb_chain_argument_list(ArgumentList *list, Expression *expr);

Expression* crb_alloc_expression(ExpressionType type);
Expression* crb_create_identifier_expression(char *identifier);
Expression* crb_create_boolean_expression(CRB_Boolean value);
Expression* crb_create_null_expression(void);
Expression* crb_create_array_expression(ExpressionList *list);
Expression* crb_create_closure_definition(char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block);
Expression* crb_create_assign_expression(CRB_Boolean is_final, Expression *left, AssignmentOperator operator, Expression *operand);
Expression* crb_create_function_call_expression(Expression *function, ArgumentList *argument);
ExpressionList* crb_create_expression_list(Expression *expression);
ExpressionList* crb_chain_expression_list(ExpressionList* list, Expression *expr);

CRB_ParameterList* crb_create_parameter(char *identifier);
CRB_ParameterList* crb_chain_parameter(CRB_ParameterList *list, char *identifier);

IdentifierList* crb_create_global_identifier(char *identifier);
IdentifierList* crb_chain_identifier(IdentifierList *list, char *identifier);

Statement* crb_create_global_statement(IdentifierList *identifier_list);
Statement* crb_create_expression_statement(Expression *expression);
StatementList* crb_create_statement_list(Statement *statement);
StatementList* crb_chain_statement_list(StatementList *list, Statement *statement);

CRB_Block* crb_create_block(StatementList *statement_list);

#endif //TLANG_TCODE_H
