//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include <wchar.h>
#include <limits.h>
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

// char
int CRB_mbstowcs_len(const char *src) {
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    int src_idx, dest_idx;
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
        if (status < 0) {
            return status;
        }
        dest_idx++;
        src_idx += status;
    }

    return dest_idx;
}

void CRB_mbstowcs(const char *src, CRB_Char *dest) {
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(&dest[dest_idx], &src[src_idx], MB_LEN_MAX, &ps);
        dest_idx++;
        src_idx += status;
    }
    dest[dest_idx] = L'\0';
}
