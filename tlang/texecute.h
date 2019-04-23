//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TEXECUTE_H
#define TLANG_TEXECUTE_H


StatementResult crb_execute_statement_list_with_pos(CRB_Interpreter *inter, CRB_LocalEnvironment *env, struct lnode* last_pos);
StatementResult crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env, StatementList *list);

#endif //TLANG_TEXECUTE_H
