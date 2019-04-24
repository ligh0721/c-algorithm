//
// Created by t5w0rd on 2019-04-23.
//

#ifndef TLANG_TOBJECT_H
#define TLANG_TOBJECT_H

#include <algorithm_tpl.h>
#include <array_tpl.h>
#include <rbtree_tpl.h>
#include <link_tpl.h>
#include "ttypedef.h"
#include "tlang.h"


COMPARE_DEFINE(CRB_Value)
TRAVERSE_DEFINE(CRB_Value)
ARRAY_DECLARE(CRB_Value)

COMPARE_DEFINE(AssocMember)
TRAVERSE_DEFINE(AssocMember)
RBTREE_DECLARE(AssocMember)

#endif //TLANG_TOBJECT_H
