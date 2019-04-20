//
// Created by t5w0rd on 19-4-20.
//

#include "tstack.h"


inline void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer) {
    inter->stack.stack_pointer = stack_pointer;
}
