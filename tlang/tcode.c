//
// Created by t5w0rd on 19-4-20.
//

#include <stddef.h>
#include "tinterpreter.h"
#include "tmisc.h"
#include "terror.h"
#include "tcode.h"


static inline LLIST* crb_create_list(const void* first) {
    static ALLOCATOR allocator = {crb_malloc, NULL};
    LLIST* ret = open_llist_with_allocator(allocator);
    llist_push_back(ret, ptr_value((void*)first));
    return ret;
}

static inline LLIST* crb_list_push_back(LLIST* list, const void* item) {
    llist_push_back(list, ptr_value((void*)item));
    return list;
}

ArgumentList* crb_create_argument_list(Expression *expression) {
    return crb_create_list(expression);
}

ArgumentList* crb_chain_argument_list(ArgumentList *list, Expression *expr) {
    return crb_list_push_back(list, expr);
}

Expression* crb_alloc_expression(ExpressionType type) {
    printf("@@ExpressionType: %d\n", (int)type);
    Expression* exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;
    return exp;
}

Expression* crb_create_identifier_expression(const char *identifier) {
    Expression* exp = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;
    return exp;
}

Expression* crb_create_boolean_expression(CRB_Boolean value) {
    Expression* exp = crb_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;
    return exp;
}

Expression* crb_create_null_expression(void) {
    Expression* exp = crb_alloc_expression(NULL_EXPRESSION);
    return exp;
}

Expression* crb_create_array_expression(ExpressionList *list) {
    Expression* exp = crb_alloc_expression(ARRAY_EXPRESSION);
    exp->u.array_literal = list;
    return exp;
}

Expression* crb_create_assign_expression(CRB_Boolean is_final, Expression *left, AssignmentOperator operator, Expression *operand) {
    Expression* exp = crb_alloc_expression(ASSIGN_EXPRESSION);
    if (is_final) {
        if (left->type == INDEX_EXPRESSION) {
            crb_compile_error(ARRAY_ELEMENT_CAN_NOT_BE_FINAL_ERR, CRB_MESSAGE_ARGUMENT_END);
        } else if (operator != NORMAL_ASSIGN) {
            crb_compile_error(COMPLEX_ASSIGNMENT_OPERATOR_TO_FINAL_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
    }
    exp->u.assign_expression.is_final = is_final;
    exp->u.assign_expression.left = left;
    exp->u.assign_expression.operator = operator;
    exp->u.assign_expression.operand = operand;
    return exp;
}

/*
 * 创建函数调用表达式
 */
Expression* crb_create_function_call_expression(Expression *function, ArgumentList *argument) {
    Expression* exp = crb_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.function = function;
    exp->u.function_call_expression.argument = argument;
    return exp;
}

/*
 * 创建表达式列表
 */
ExpressionList* crb_create_expression_list(Expression *expression) {
    return crb_create_list(expression);
}

ExpressionList* crb_chain_expression_list(ExpressionList *list, Expression *expr) {
    return crb_list_push_back(list, expr);
}

/*
 * 创建函数定义
 */
static CRB_FunctionDefinition* create_function_definition(const char *identifier, CRB_ParameterList *parameter_list, CRB_Boolean is_closure, CRB_Block *block) {
    CRB_FunctionDefinition* f = crb_malloc(sizeof(CRB_FunctionDefinition));
    f->name = identifier;
    f->type = CRB_CROWBAR_FUNCTION_DEFINITION;
    f->is_closure = is_closure;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;
    return f;
}

/*
 * 创建命名函数定义
 */
void crb_function_define(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block) {
    if (crb_search_function_in_compile(identifier)) {
        crb_compile_error(FUNCTION_MULTIPLE_DEFINE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", identifier, CRB_MESSAGE_ARGUMENT_END);
        return;
    }
    CRB_FunctionDefinition* f = create_function_definition(identifier, parameter_list, CRB_FALSE, block);
    CRB_Interpreter* inter = crb_get_current_interpreter();
    rbtree_set(inter->functions, ptr_value(f));
//    f->next = inter->function_list;
//    inter->function_list = f;
}

/*
 * 创建匿名函数定义
 */
Expression* crb_create_closure_definition(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block) {
    Expression* exp = crb_alloc_expression(CLOSURE_EXPRESSION);
    exp->u.closure.function_definition = create_function_definition(identifier, parameter_list, CRB_TRUE, block);
    return exp;
}

CRB_ParameterList* crb_create_parameter(const char *identifier) {
    return crb_create_list(identifier);
}

CRB_ParameterList* crb_chain_parameter(CRB_ParameterList *list, const char *identifier) {
    return crb_list_push_back(list, identifier);
}

IdentifierList* crb_create_global_identifier(const char *identifier) {
    return crb_create_list(identifier);
}

IdentifierList* crb_chain_identifier(IdentifierList *list, const char *identifier) {
    return crb_list_push_back(list, identifier);
}

static Statement* alloc_statement(StatementType type) {
    Statement* st = crb_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = crb_get_current_interpreter()->current_line_number;
    return st;
}

Statement* crb_create_global_statement(IdentifierList *identifier_list) {
    Statement* st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;
    return st;
}

Statement* crb_create_expression_statement(Expression *expression) {
    Statement* st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;
    return st;
}

StatementList* crb_create_statement_list(Statement *statement) {
    return crb_create_list(statement);
}

StatementList* crb_chain_statement_list(StatementList *list, Statement *statement) {
    return list == NULL ? crb_create_statement_list(statement) : crb_list_push_back(list, statement);
}

CRB_Block* crb_create_block(StatementList *statement_list) {
    CRB_Block* block = crb_malloc(sizeof(CRB_Block));
    block->statement_list = statement_list;
    return block;
}
