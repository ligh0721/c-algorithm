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
Expression* crb_create_assoc_literal_expression(AssocExpressionList* list);
Expression* crb_create_minus_expression(Expression *operand);
Expression* crb_create_logical_not_expression(Expression *operand);
Expression* crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right);
Expression* crb_create_assign_expression(CRB_Boolean is_final, Expression *left, AssignmentOperator operator, Expression *operand);
Expression* crb_create_comma_expression(Expression *left, Expression *right);
Expression* crb_create_index_expression(Expression *array, Expression *index);
Expression* crb_create_slice_expression(Expression *array, Expression *begin, Expression *end);
Expression* crb_create_member_expression(Expression *expression, const char *member_name);
Expression* crb_create_inc_dec_expression(Expression *operand, ExpressionType inc_or_dec);
Expression* crb_create_function_call_expression(Expression *function, ArgumentList *argument);

ExpressionList* crb_create_expression_list(Expression *expression);
ExpressionList* crb_chain_expression_list(ExpressionList* list, Expression *expr);

AssocExpression* crb_create_assoc_expression(CRB_Boolean is_final, const char* member_name, Expression* expr);
AssocExpressionList* crb_create_assoc_expression_list(AssocExpression* expr);
AssocExpressionList* crb_chain_assoc_expression_list(AssocExpressionList* list, AssocExpression* expr);

void crb_function_define(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block);
Expression* crb_create_closure_definition(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block);

CRB_ParameterList* crb_create_parameter_list(const char *identifier);
CRB_ParameterList* crb_chain_parameter_list(CRB_ParameterList *list, const char *identifier);

ArgumentList* crb_create_argument_list(Expression *expression);
ArgumentList* crb_chain_argument_list(ArgumentList *list, Expression *expr);

IdentifierList* crb_create_global_identifier_list(const char *identifier);
IdentifierList* crb_chain_global_identifier_list(IdentifierList *list, const char *identifier);

Statement* crb_create_global_statement(IdentifierList *identifier_list);
Statement* crb_create_expression_statement(Expression *expression);
Statement* crb_create_return_statement(Expression *expression);

StatementList* crb_create_statement_list(Statement *statement);
StatementList* crb_chain_statement_list(StatementList *list, Statement *statement);

CRB_Block* crb_create_block(StatementList *statement_list);

Statement* crb_create_if_statement(Expression *condition, CRB_Block *then_block, ElifList *elif_list, CRB_Block *else_block);
Elif* crb_create_elif(Expression *expr, CRB_Block *block);
ElifList* crb_create_elif_list(Elif* elif);
ElifList* crb_chain_elif_list(ElifList* list, Elif* elif);

Statement* crb_create_while_statement(const char *label, Expression *condition, CRB_Block *block);
Statement* crb_create_for_statement(const char *label, Expression *init, Expression *cond, Expression *post, CRB_Block *block);
Statement* crb_create_foreach_statement(const char *label, const char *variable, Expression *collection, CRB_Block *block);
Statement* crb_create_break_statement(const char* label);
Statement* crb_create_continue_statement(const char *label);

Statement* crb_create_try_statement(CRB_Block *try_block, const char *exception, CRB_Block *catch_block, CRB_Block *finally_block);
Statement* crb_create_throw_statement(Expression *expression);

#endif //TLANG_TCODE_H
