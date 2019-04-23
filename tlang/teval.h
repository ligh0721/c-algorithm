//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TEVAL_H
#define TLANG_TEVAL_H


// stack operation
void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer);
int crb_get_stack_pointer(CRB_Interpreter *inter);

// evel expression
CRB_Value crb_eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *operand);
CRB_Value crb_eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right);
CRB_Value* crb_get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *identifier);
CRB_Value crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

#endif //TLANG_TEVAL_H
