//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TSTACK_H
#define TLANG_TSTACK_H

#include "tinterpreter.h"

void push_value(CRB_Interpreter *inter, CRB_Value *value);
CRB_Value pop_value(CRB_Interpreter *inter);
CRB_Value* peek_stack(CRB_Interpreter *inter, int index);

void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer);

void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size);

#endif //TLANG_TSTACK_H
