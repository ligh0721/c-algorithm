//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TMISC_H
#define TLANG_TMISC_H

#include "tinterpreter.h"


void* crb_malloc(size_t size);

void crb_vstr_clear(VString *v);
void crb_vstr_append_string(VString *v, CRB_Char *str);
void crb_vstr_append_character(VString *v, CRB_Char ch);

#endif //TLANG_TMISC_H
