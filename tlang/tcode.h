//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TCODE_H
#define TLANG_TCODE_H

#include "tinterpreter.h"


Expression* crb_alloc_expression(ExpressionType type);
Expression* crb_create_assign_expression(CRB_Boolean is_final, Expression *left, AssignmentOperator operator, Expression *operand);

#endif //TLANG_TCODE_H
