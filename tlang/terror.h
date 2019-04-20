//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TERROR_H
#define TLANG_TERROR_H

#include "tinterpreter.h"


void crb_compile_error(CompileError id, ...);
void crb_runtime_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, RuntimeError id, ...);

#endif //TLANG_TERROR_H
