//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TMISC_H
#define TLANG_TMISC_H


void* crb_malloc(size_t size);
void* crb_execute_malloc(CRB_Interpreter *inter, size_t size);
const char* crb_get_operator_string(ExpressionType type);
const char* CRB_get_type_name(CRB_ValueType type);

void crb_vstr_clear(VString *v);
void crb_vstr_append_string(VString *v, const CRB_Char *str);
void crb_vstr_append_character(VString *v, CRB_Char ch);

Variable* crb_search_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier);

CRB_FunctionDefinition* crb_search_function_in_compile(const char *name);
FakeMethodDefinition* crb_search_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_FakeMethod *fm);

#endif //TLANG_TMISC_H
