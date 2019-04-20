//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TMEMORY_H
#define TLANG_TMEMORY_H

#include <stdio.h>
#include "tlang.h"


typedef union Header_tag Header;

struct MEM_Controller_tag {
    FILE        *error_fp;
    MEM_ErrorHandler    error_handler;
    MEM_FailMode        fail_mode;
    Header      *block_header;
};

#endif //TLANG_TMEMORY_H
