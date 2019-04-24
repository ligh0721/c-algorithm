//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_THEAP_H
#define TLANG_THEAP_H


// string object
CRB_Object* crb_literal_to_crb_string_i(CRB_Interpreter *inter, CRB_Char *str);
CRB_Object* crb_create_crowbar_string_i(CRB_Interpreter *inter, CRB_Char *str);

// array object
CRB_Object* crb_create_array_i(CRB_Interpreter *inter, int size);

// assoc
CRB_Object* crb_create_assoc_i(CRB_Interpreter *inter);

// scope chain
CRB_Object* crb_create_scope_chain(CRB_Interpreter *inter, CRB_Object* frame, CRB_Object* next);

// native pointer
CRB_Object* crb_create_native_pointer_i(CRB_Interpreter *inter, void *pointer, CRB_NativePointerInfo *info);

// gc
void crb_garbage_collect(CRB_Interpreter* inter);

#endif //TLANG_THEAP_H
