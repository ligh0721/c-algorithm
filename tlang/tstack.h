//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TSTACK_H
#define TLANG_TSTACK_H

#include "tinterpreter.h"


void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer);

void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size);

#endif //TLANG_TSTACK_H
