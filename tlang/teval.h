//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TEVAL_H
#define TLANG_TEVAL_H

#include "tinterpreter.h"


CRB_Value crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

#endif //TLANG_TEVAL_H
