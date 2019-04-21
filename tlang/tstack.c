//
// Created by t5w0rd on 19-4-20.
//

#include "tstack.h"


inline void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer) {
    inter->stack.stack_pointer = stack_pointer;
}

static inline void shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    inter->stack.stack_pointer -= shrink_size;
}

void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    shrink_stack(inter, shrink_size);
}
