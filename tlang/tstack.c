//
// Created by t5w0rd on 19-4-20.
//

#include "tstack.h"


void push_value(CRB_Interpreter *inter, CRB_Value *value) {
    DBG_assert(inter->stack.stack_pointer <= inter->stack.stack_alloc_size, ("stack_pointer..%d, stack_alloc_size..%d\n", inter->stack.stack_pointer, inter->stack.stack_alloc_size));

    if (inter->stack.stack_pointer == inter->stack.stack_alloc_size) {
        inter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        inter->stack.stack = MEM_realloc(inter->stack.stack, sizeof(CRB_Value) * inter->stack.stack_alloc_size);
    }
    inter->stack.stack[inter->stack.stack_pointer] = *value;
    inter->stack.stack_pointer++;
}

CRB_Value pop_value(CRB_Interpreter *inter) {
    CRB_Value ret = inter->stack.stack[inter->stack.stack_pointer-1];
    inter->stack.stack_pointer--;
    return ret;
}

CRB_Value* peek_stack(CRB_Interpreter *inter, int index) {
    return &inter->stack.stack[inter->stack.stack_pointer - index - 1];
}

inline void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer) {
    inter->stack.stack_pointer = stack_pointer;
}

static inline void shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    inter->stack.stack_pointer -= shrink_size;
}

void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    shrink_stack(inter, shrink_size);
}
