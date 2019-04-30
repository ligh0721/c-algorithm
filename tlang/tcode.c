//
// Created by t5w0rd on 19-4-20.
//

#include <stddef.h>
#include "tinterpreter.h"
#include "tmisc.h"
#include "terror.h"
#include "tcode.h"
#include "teval.h"


/*
 * 通用创建列表项并追加列表项
 * 构成语句，内存在关闭解释器时统一释放
 */
static inline LLIST* create_list(const void *first) {
    static ALLOCATOR allocator = {crb_malloc, NULL};
    LLIST* ret = open_llist_with_allocator(allocator);
    if (first == NULL) {
        return ret;
    }
    llist_push_back(ret, ptr_value((void*)first));
    return ret;
}

/*
 * 通用追加列表项
 */
static inline LLIST* chain_list(LLIST *list, const void *item) {
    if (item == NULL) {
        return list;
    }
    llist_push_back(list, ptr_value((void*)item));
    return list;
}

/*
 * 分配表达式内存
 */
Expression* crb_alloc_expression(ExpressionType type) {
    Expression* exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;
    return exp;
}

/*
 * 创建只含有标识符的表达式
 */
Expression* crb_create_identifier_expression(const char *identifier) {
    Expression* exp = crb_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier = identifier;
    return exp;
}

/*
 * 创建布尔值字面量表达式
 */
Expression* crb_create_boolean_expression(CRB_Boolean value) {
    Expression* exp = crb_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;
    return exp;
}

/*
 * 创建NULL字面量表达式
 */
Expression* crb_create_null_expression(void) {
    Expression* exp = crb_alloc_expression(NULL_EXPRESSION);
    return exp;
}

/*
 * 创建数组字面量表达式
 */
Expression* crb_create_array_expression(ExpressionList* list) {
    Expression* exp = crb_alloc_expression(ARRAY_EXPRESSION);
    exp->u.array_literal = list;
    return exp;
}

/*
 * 创建对象(关联数组)字面量表达式
 */
Expression* crb_create_assoc_literal_expression(AssocExpressionList* list) {
    Expression* exp = crb_alloc_expression(ASSOC_EXPRESSION);
    exp->u.assoc_literal = list;
    return exp;
}

/*
 * 将值类型转换成字面量表达式
 */
static void set_expression(Expression *expr, CRB_Value *v) {
    if (v->type == CRB_INT_VALUE) {
        expr->type = INT_EXPRESSION;
        expr->u.int_value = v->u.int_value;
    } else if (v->type == CRB_FLOAT_VALUE) {
        expr->type = DOUBLE_EXPRESSION;
        expr->u.float_value = v->u.float_value;
    } else if (v->type == CRB_BOOLEAN_VALUE) {
        expr->type = BOOLEAN_EXPRESSION;
        expr->u.boolean_value = v->u.boolean_value;
    }
}

Expression* crb_create_minus_expression(Expression *operand) {
    if (operand->type == INT_EXPRESSION || operand->type == DOUBLE_EXPRESSION) {
        CRB_Value v;
        crb_eval_minus_expression(crb_get_current_interpreter(), NULL, operand, &v);
        /* Notice! Overwriting operand expression. */
        set_expression(operand, &v);
        return operand;
    } else {
        Expression* exp = crb_alloc_expression(MINUS_EXPRESSION);
        exp->u.minus_expression = operand;
        return exp;
    }
}

/*
 * 创建逻辑非表达式
 */
Expression* crb_create_logical_not_expression(Expression *operand) {
    Expression* exp = crb_alloc_expression(LOGICAL_NOT_EXPRESSION);
    exp->u.logical_not = operand;
    return exp;
}

/*
 * 创建二元表达式
 */
Expression* crb_create_binary_expression(ExpressionType operator, Expression *left, Expression *right) {
    if ((left->type == INT_EXPRESSION || left->type == DOUBLE_EXPRESSION) && (right->type == INT_EXPRESSION || right->type == DOUBLE_EXPRESSION)) {
        CRB_Value v;
        crb_eval_binary_expression(crb_get_current_interpreter(), NULL, operator, left, right, &v);
        /* Overwriting left hand expression. */
        set_expression(left, &v);
        return left;
    } else {
        Expression* exp = crb_alloc_expression(operator);
        exp->u.binary_expression.left = left;
        exp->u.binary_expression.right = right;
        return exp;
    }
}

/*
 * 创建赋值表达式
 */
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
 * 创建逗号表达式
 */
Expression* crb_create_comma_expression(Expression *left, Expression *right) {
    Expression* exp = crb_alloc_expression(COMMA_EXPRESSION);
    exp->u.comma.left = left;
    exp->u.comma.right = right;
    return exp;
}

/*
 * 创建索引数组表达式
 */
Expression* crb_create_index_expression(Expression *array, Expression *index) {
    Expression* exp = crb_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;
    return exp;
}

/*
 * 创建数组切片表达式
 */
Expression* crb_create_slice_expression(Expression *array, Expression *begin, Expression *end) {
    Expression* exp = crb_alloc_expression(SLICE_EXPRESSION);
    exp->u.slice_expression.array = array;
    exp->u.slice_expression.start = begin;
    exp->u.slice_expression.end = end;
    return exp;
}

/*
 * 创建访问成员表达式
 */
Expression* crb_create_member_expression(Expression *expression, const char *member_name) {
    Expression* exp = crb_alloc_expression(MEMBER_EXPRESSION);
    exp->u.member_expression.expression = expression;
    exp->u.member_expression.member_name = member_name;
    return exp;
}

/*
 * 创建自增自减表达式
 */
Expression* crb_create_inc_dec_expression(Expression *operand, ExpressionType inc_or_dec) {
    Expression* exp = crb_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;
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
    return create_list(expression);
}

ExpressionList* crb_chain_expression_list(ExpressionList *list, Expression *expr) {
    return chain_list(list, expr);
}

/*
 * 创建关联数组字面量成员项
 */
AssocExpression* crb_create_assoc_expression(CRB_Boolean is_final, const char* member_name, Expression* expr) {
    AssocExpression* assoc_expr = crb_malloc(sizeof(AssocExpression));
    assoc_expr->member_name = member_name;
    assoc_expr->expression = expr;
    assoc_expr->is_final = is_final;
    return assoc_expr;
}
/*
 * 创建关联数组字面量表达式列表
 */
AssocExpressionList* crb_create_assoc_expression_list(AssocExpression* expr) {
    return create_list(expr);
}

AssocExpressionList* crb_chain_assoc_expression_list(AssocExpressionList* list, AssocExpression* expr) {
    return chain_list(list, expr);
}

/*
 * 创建函数定义
 */
static inline CRB_FunctionDefinition* create_function_definition(CRB_Module* module, const char *identifier, CRB_ParameterList *parameter_list, CRB_Boolean is_closure, CRB_Block *block) {
    CRB_FunctionDefinition* f = crb_malloc(sizeof(CRB_FunctionDefinition));
    f->name = identifier;
    f->type = CRB_CROWBAR_FUNCTION_DEFINE;
    f->is_closure = is_closure;
    f->u.crowbar_f.parameter = parameter_list;
    f->u.crowbar_f.block = block;
    f->module = module;
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
    CRB_Interpreter* inter = crb_get_current_interpreter();
    CRB_FunctionDefinition* f = create_function_definition(inter->current_module, identifier, parameter_list, CRB_FALSE, block);
    RBTREE* global_funcs = CRB_global_funcs(inter, inter->current_module);
    rbtree_set(global_funcs, ptr_value(f));
}

/*
 * 创建匿名函数定义表达式
 */
Expression* crb_create_closure_definition(const char *identifier, CRB_ParameterList *parameter_list, CRB_Block *block) {
    Expression* exp = crb_alloc_expression(CLOSURE_EXPRESSION);
    CRB_Interpreter* inter = crb_get_current_interpreter();
    exp->u.closure.function_definition = create_function_definition(inter->current_module, identifier, parameter_list, CRB_TRUE, block);
    return exp;
}

/*
 * 创建形参列表
 */
CRB_ParameterList* crb_create_parameter_list(const char *identifier) {
    return create_list(identifier);
}

CRB_ParameterList* crb_chain_parameter_list(CRB_ParameterList *list, const char *identifier) {
    return chain_list(list, identifier);
}

/*
 * 创建实参列表
 */
ArgumentList* crb_create_argument_list(Expression *expression) {
    return create_list(expression);
}

ArgumentList* crb_chain_argument_list(ArgumentList *list, Expression *expr) {
    return chain_list(list, expr);
}

/*
 * 创建全局变量引用列表
 */
IdentifierList* crb_create_global_identifier_list(const char *identifier) {
    return create_list(identifier);
}

IdentifierList* crb_chain_global_identifier_list(IdentifierList *list, const char *identifier) {
    return chain_list(list, identifier);
}

static inline Statement* alloc_statement(StatementType type) {
    Statement* st = crb_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = crb_get_current_interpreter()->current_line_number;
    return st;
}

/*
 * 创建全局引用语句
 */
Statement* crb_create_global_statement(IdentifierList *identifier_list) {
    Statement* st = alloc_statement(GLOBAL_STATEMENT);
    st->u.global_s.identifier_list = identifier_list;
    return st;
}

/*
 * 创建表达式语句
 */
Statement* crb_create_expression_statement(Expression *expression) {
    if (expression == NULL) {
        return NULL;
    }
    Statement* st = alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;
    return st;
}

Statement* crb_create_return_statement(Expression *expression) {
    Statement* st = alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;
    return st;
}

/*
 * 创建语句列表
 */
StatementList* crb_create_statement_list(Statement *statement) {
    return create_list(statement);
}

StatementList* crb_chain_statement_list(StatementList *list, Statement *statement) {
    return list == NULL ? crb_create_statement_list(statement) : chain_list(list, statement);
}

/*
 * 创建语句块
 */
CRB_Block* crb_create_block(StatementList *statement_list) {
    CRB_Block* block = crb_malloc(sizeof(CRB_Block));
    block->statement_list = statement_list;
    return block;
}

/*
 * 创建if语句
 */
Statement* crb_create_if_statement(Expression *condition, CRB_Block *then_block, ElifList *elif_list, CRB_Block *else_block) {
    Statement* st = alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elif_list = elif_list;
    st->u.if_s.else_block = else_block;
    return st;
}

static inline Elif* alloc_elif(Expression* expr, CRB_Block* block) {
    Elif* elif = crb_malloc(sizeof(Elif));
    elif->condition = expr;
    elif->block = block;
    return elif;
}

/*
 * 创建elif部分
 */
Elif* crb_create_elif(Expression *expr, CRB_Block *block) {
    Elif* elif = alloc_elif(expr, block);
    return elif;
}

/*
 * 创建elif列表
 */
ElifList* crb_create_elif_list(Elif* elif) {
    return create_list(elif);
}

ElifList* crb_chain_elif_list(ElifList* list, Elif* elif) {
    return chain_list(list, elif);
}

/*
 * 创建while语句
 */
Statement* crb_create_while_statement(const char *label, Expression *condition, CRB_Block *block) {
    Statement* st = alloc_statement(WHILE_STATEMENT);
    st->u.while_s.label = label;
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;
    return st;
}

/*
 * 创建for语句
 */
Statement* crb_create_for_statement(const char *label, Expression *init, Expression *cond, Expression *post, CRB_Block *block) {
    Statement* st = alloc_statement(FOR_STATEMENT);
    st->u.for_s.label = label;
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;
    return st;
}

/*
 * 创建foreach语句
 */
Statement* crb_create_foreach_statement(const char *label, const char *variable, Expression *collection, CRB_Block *block) {
    Statement* st = alloc_statement(FOREACH_STATEMENT);
    st->u.foreach_s.label = label;
    st->u.foreach_s.variable = variable;
    st->u.foreach_s.collection = collection;
    st->u.for_s.block = block;
    return st;
}

/*
 * 创建break语句
 */
Statement* crb_create_break_statement(const char* label) {
    Statement* st = alloc_statement(BREAK_STATEMENT);
    st->u.break_s.label = label;
    return st;
}

/*
 * 创建continue语句
 */
Statement* crb_create_continue_statement(const char *label) {
    Statement* st = alloc_statement(CONTINUE_STATEMENT);
    st->u.continue_s.label = label;
    return st;
}

/*
 * 创建try语句
 */
Statement* crb_create_try_statement(CRB_Block *try_block, const char *exception, CRB_Block *catch_block, CRB_Block *finally_block) {
    Statement* st = alloc_statement(TRY_STATEMENT);
    st->u.try_s.try_block = try_block;
    st->u.try_s.catch_block = catch_block;
    st->u.try_s.exception = exception;
    st->u.try_s.finally_block = finally_block;
    return st;
}

/*
 * 创建throw语句
 */
Statement* crb_create_throw_statement(Expression *expression) {
    Statement* st = alloc_statement(THROW_STATEMENT);
    st->u.throw_s.exception = expression;
    return st;
}

/*
 * 创建call语句
 */
Statement* crb_create_import_statement(Expression* name) {
    Statement* st = alloc_statement(IMPORT_STATEMENT);
    st->u.import_s.name = name;
    return st;
}
