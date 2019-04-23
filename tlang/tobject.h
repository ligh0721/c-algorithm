//
// Created by t5w0rd on 2019-04-23.
//

#ifndef TLANG_TOBJECT_H
#define TLANG_TOBJECT_H

#include <algorithm_tpl.h>
#include <array_tpl.h>
#include <rbtree_tpl.h>
#include "ttypedef.h"
#include "tlang.h"


COMPARE_DEF(CRB_Value)
TRAVERSE_DEF(CRB_Value)
ARRAY_DECL(CRB_Value)

COMPARE_DEF(AssocMember)
TRAVERSE_DEF(AssocMember)
RBTREE_DECL(AssocMember)

#endif //TLANG_TOBJECT_H
