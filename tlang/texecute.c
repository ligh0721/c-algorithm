//
// Created by t5w0rd on 19-4-20.
//

#include "texecute.h"
#include "terror.h"
#include "tmisc.h"
#include "teval.h"


static StatementResult execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement);

static StatementResult execute_expression_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    crb_eval_expression(inter, env, statement->u.expression_s);
    return result;
}

static StatementResult execute_global_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    if (env == NULL) {
        crb_runtime_error(inter, env, statement->line_number, GLOBAL_STATEMENT_IN_TOPLEVEL_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    IdentifierList* global_identifiers = statement->u.global_s.identifier_list;
    for (struct lnode* node=llist_front_node(global_identifiers); node!=NULL; node=node->next) {
        char* identifier_name = (char*)node->value.ptr_value;
        NamedItemEntry key = {identifier_name};
        GlobalVariableRef* global_ref = env->global_variable;
//        int ok;
//        rbtree_get(env->global_variable, ptr_value(&key), &ok);
        RBNODE* parent;
        RBNODE** where = rbtree_fast_get(global_ref, ptr_value(&key), &parent);
        if (!rbtree_node_not_found(global_ref, where)) {
            continue;
        }
        Variable* variable = crb_search_global_variable(inter, identifier_name);
        if (variable == NULL) {
            crb_runtime_error(inter, env, statement->line_number, GLOBAL_VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", identifier_name, CRB_MESSAGE_ARGUMENT_END);
        }
//        rbtree_set(env->global_variable, ptr_value(variable));
        RBNODE* var_node = rbtree_open_node(global_ref, ptr_value(variable), parent);
        rbtree_fast_set(global_ref, where, var_node);
    }
    return result;
}

static StatementResult execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    switch (statement->type) {
        case EXPRESSION_STATEMENT:
            result = execute_expression_statement(inter, env, statement);
            break;
        case GLOBAL_STATEMENT:
            result = execute_global_statement(inter, env, statement);
            break;
            // TODO:
//        case IF_STATEMENT:
//            result = execute_if_statement(inter, env, statement);
//            break;
//        case WHILE_STATEMENT:
//            result = execute_while_statement(inter, env, statement);
//            break;
//        case FOR_STATEMENT:
//            result = execute_for_statement(inter, env, statement);
//            break;
//        case FOREACH_STATEMENT:
//            result = execute_foreach_statement(inter, env, statement);
//            break;
//        case RETURN_STATEMENT:
//            result = execute_return_statement(inter, env, statement);
//            break;
//        case BREAK_STATEMENT:
//            result = execute_break_statement(inter, env, statement);
//            break;
//        case CONTINUE_STATEMENT:
//            result = execute_continue_statement(inter, env, statement);
//            break;
//        case TRY_STATEMENT:
//            result = execute_try_statement(inter, env, statement);
//            break;
//        case THROW_STATEMENT:
//            result = execute_throw_statement(inter, env, statement);
//            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:   /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case...%d", statement->type));
    }

    return result;
}

struct execute_params {
    CRB_Interpreter *inter;
    CRB_LocalEnvironment *env;
    StatementResult *res;
};

static int execute_every_statement(VALUE value, void* param) {
    struct execute_params* params = (struct execute_params*)param;
    *(params->res) = execute_statement(params->inter, params->env, (Statement*)value.ptr_value);
    if (params->res->type != NORMAL_STATEMENT_RESULT) {
        return 1;
    }
    return 0;
}

StatementResult crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env, StatementList *list) {
    StatementResult ret;
    ret.type = NORMAL_STATEMENT_RESULT;
    struct execute_params params = {inter, env, &ret};
    llist_traversal(list, execute_every_statement, &params);
    return ret;
}
