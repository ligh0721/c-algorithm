//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TDEBUG_H
#define TLANG_TDEBUG_H

#include <stdio.h>
#include "tlang.h"


struct DBG_Controller_tag {
    FILE        *debug_write_fp;
    int         current_debug_level;
};

#endif //TLANG_TDEBUG_H
