//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TCODE_H
#define TLANG_TCODE_H


Expression* crb_alloc_expression(ExpressionType type);
Expression* crb_create_identifier_expression(const char *identifier);
Expression* crb_create_boolean_expression(CRB_Boolean value);
Expression* crb_create_null_expression(void);
Expression* crb_create_array_expression(ExpressionList *list);
Expression* crb_create_minus_expression(Expression *operand);
Expression* crb_create_logical_not_expression(Expression *operand);
Expression* crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right);
Expression* crb_create_assign_expression(CRB_Boolean is_final, Expression *left, AssignmentOperator operator, Expression *operand);
Expression* crb_create_comma_expression(Expression *left, Expression *right);
Expression* crb_create_index_expression(Expression *array, Expression *index);
Expression* crb_create_member_expression(Expression *expression, char *member_name);
Expression* crb_create_inc_dec_expression(Expression *operand, ExpressionType inc_or_dec);
Expression* crb_create_function_call_expression(Expression *function, ArgumentList *argument);

ExpressionList* crb_create_expression_list(Expression *expression);
ExpressionList* crb_chain_expression_list(ExpressionList* list, Expression *expr);

void crb_function_define(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block);
Expression* crb_create_closure_definition(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block);

CRB_ParameterList* crb_create_parameter(const char *identifier);
CRB_ParameterList* crb_chain_parameter(CRB_ParameterList *list, const char *identifier);

ArgumentList* crb_create_argument_list(Expression *expression);
ArgumentList* crb_chain_argument_list(ArgumentList *list, Expression *expr);

IdentifierList* crb_create_global_identifier(const char *identifier);
IdentifierList* crb_chain_identifier(IdentifierList *list, const char *identifier);

Statement* crb_create_global_statement(IdentifierList *identifier_list);
Statement* crb_create_expression_statement(Expression *expression);
Statement* crb_create_return_statement(Expression *expression);

StatementList* crb_create_statement_list(Statement *statement);
StatementList* crb_chain_statement_list(StatementList *list, Statement *statement);

CRB_Block* crb_create_block(StatementList *statement_list);

Statement* crb_create_if_statement(Expression *condition, CRB_Block *then_block, Elsif *elsif_list, CRB_Block *else_block);
Elsif* crb_create_elsif(Expression *expr, CRB_Block *block);
Elsif* crb_chain_elsif_list(Elsif *list, Elsif *add);

#endif //TLANG_TCODE_H
