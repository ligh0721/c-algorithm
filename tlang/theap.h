//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_THEAP_H
#define TLANG_THEAP_H

#include "tinterpreter.h"


CRB_Object* crb_create_native_pointer_i(CRB_Interpreter *inter, void *pointer, CRB_NativePointerInfo *info);
void crb_garbage_collect(CRB_Interpreter* inter);

#endif //TLANG_THEAP_H
