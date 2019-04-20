//
// Created by t5w0rd on 19-4-20.
//

#include "tcode.h"
#include "tmisc.h"


Expression* crb_alloc_expression(ExpressionType type) {
    printf("ExpressionType: %d\n", (int)type);
    Expression* exp = crb_malloc(sizeof(Expression));
    exp->type = type;
    exp->line_number = crb_get_current_interpreter()->current_line_number;
    return exp;
}
