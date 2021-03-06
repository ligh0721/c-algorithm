//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TNATIVE_H
#define TLANG_TNATIVE_H

void crb_init_native_const_values();
void crb_add_std_fp(CRB_Interpreter *inter);
void crb_add_native_functions(CRB_Interpreter* inter);
FakeMethodDefinition* crb_add_fake_method(CRB_Interpreter *interpreter, ObjectType type, const char *name, int param_count, FakeMethodFunc *func);
void crb_add_fake_methods(CRB_Interpreter *inter);

#endif //TLANG_TNATIVE_H
