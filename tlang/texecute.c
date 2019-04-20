//
// Created by t5w0rd on 19-4-20.
//

#include "texecute.h"


static StatementResult execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result = {};
    // TODO:
    return result;
}

StatementResult crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env, StatementList *list) {
    StatementResult ret;
    ret.type = NORMAL_STATEMENT_RESULT;
    for (StatementList* pos = list; pos; pos = pos->next) {
        ret = execute_statement(inter, env, pos->statement);
        if (ret.type != NORMAL_STATEMENT_RESULT) {
            goto FUNC_END;
        }
    }
    FUNC_END:
    return ret;
}
