//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include "tmisc.h"


void* crb_malloc(size_t size) {
    CRB_Interpreter* inter = crb_get_current_interpreter();
    return MEM_storage_malloc(inter->interpreter_storage, size);
}

void* crb_execute_malloc(CRB_Interpreter *inter, size_t size) {
    return MEM_storage_malloc(inter->execute_storage, size);
}


void crb_vstr_clear(VString *v) {
    v->string = NULL;
}



CRB_Value* CRB_add_global_variable(CRB_Interpreter *inter, char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    Variable* new_var = crb_execute_malloc(inter, sizeof(Variable));
    new_var->is_final = is_final;
    new_var->name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(new_var->name, identifier);
    new_var->value = *value;
    rbtree_set(inter->variables, ptr_value(new_var));
//    new_var->next = inter->variable;
//    inter->variable = new_var;

    return &new_var->value;
}





void* CRB_object_get_native_pointer(CRB_Object *obj) {
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT, ("obj->type..%d\n", obj->type));
    return obj->u.native_pointer.pointer;
}
