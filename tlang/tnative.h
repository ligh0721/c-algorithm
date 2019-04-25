//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TNATIVE_H
#define TLANG_TNATIVE_H


void crb_add_native_functions(CRB_Interpreter* inter);
void crb_add_std_fp(CRB_Interpreter *inter);

FakeMethodTable* crb_search_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_FakeMethod *fm);

#endif //TLANG_TNATIVE_H
