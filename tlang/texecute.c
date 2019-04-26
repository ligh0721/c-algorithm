//
// Created by t5w0rd on 19-4-20.
//

#include "tinterpreter.h"
#include "texecute.h"
#include "terror.h"
#include "tmisc.h"
#include "teval.h"


static StatementResult execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement);

/*
 * 执行表达式语句
 */
static StatementResult execute_expression_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    crb_eval_expression(inter, env, statement->u.expression_s);
    return result;
}

/*
 * 执行全局引用语句
 */
static StatementResult execute_global_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    if (env == NULL) {
        crb_runtime_error(inter, env, statement->line_number, GLOBAL_STATEMENT_IN_TOPLEVEL_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    IdentifierList* global_identifiers = statement->u.global_s.identifier_list;
    for (struct lnode* node=global_identifiers?llist_front_node(global_identifiers):NULL; node!=NULL; node=node->next) {
        char* identifier_name = (char*)node->value.ptr_value;
        NamedItemEntry key = {identifier_name};
        GlobalVariableRef* global_ref = env->global_var_refs;
        RBNODE* parent;
        RBNODE** where = rbtree_fast_get(global_ref, ptr_value(&key), &parent);
        if (!rbtree_node_not_found(global_ref, where)) {
            continue;
        }
        Variable* variable = crb_search_global_variable(inter, identifier_name);
        if (variable == NULL) {
            crb_runtime_error(inter, env, statement->line_number, GLOBAL_VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", identifier_name, CRB_MESSAGE_ARGUMENT_END);
        }
//        rbtree_set(env->global_var_refs, ptr_value(variable));
        RBNODE* var_node = rbtree_open_node(global_ref, ptr_value(variable), parent);
        rbtree_fast_set(global_ref, where, var_node);
    }
    return result;
}

/*
 * 执行return语句
 */
static StatementResult execute_return_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = RETURN_STATEMENT_RESULT;
    if (statement->u.return_s.return_value) {
        result.u.return_value = crb_eval_expression(inter, env, statement->u.return_s.return_value);
    } else {
        result.u.return_value.type = CRB_NULL_VALUE;
    }
    return result;
}

/*
 * 执行elif部分
 */
static StatementResult execute_elif(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ElifList *elif_list, CRB_Boolean *executed) {
    *executed = CRB_FALSE;
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    for (struct lnode* node=elif_list?llist_front_node(elif_list):NULL; node!=NULL; node=node->next) {
        Elif* elif = (Elif*)node->value.ptr_value;
        CRB_Value cond = crb_eval_expression(inter, env, elif->condition);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, elif->condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (cond.u.boolean_value) {
            result = crb_execute_statement_list(inter, env, elif->block->statement_list);
            *executed = CRB_TRUE;
            if (result.type != NORMAL_STATEMENT_RESULT) {
                goto FUNC_END;
            }
        }
    }

    FUNC_END:
    return result;
}

/*
 * 执行if语句
 */
static StatementResult execute_if_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    CRB_Value cond = crb_eval_expression(inter, env, statement->u.if_s.condition);
    if (cond.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, statement->u.if_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));

    if (cond.u.boolean_value) {
        result = crb_execute_statement_list(inter, env, statement->u.if_s.then_block->statement_list);
    } else {
        CRB_Boolean elif_executed;
        result = execute_elif(inter, env, statement->u.if_s.elif_list, &elif_executed);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            goto FUNC_END;
        }
        if (!elif_executed && statement->u.if_s.else_block) {
            result = crb_execute_statement_list(inter, env, statement->u.if_s.else_block->statement_list);
        }
    }

    FUNC_END:
    return result;
}

static StatementResultType compare_labels(const char *result_label, const char *loop_label, StatementResultType current_result) {
    if (result_label == NULL) {
        return NORMAL_STATEMENT_RESULT;
    }
    if (loop_label && !strcmp(result_label, loop_label)) {
        return NORMAL_STATEMENT_RESULT;
    } else {
        return current_result;
    }
}

/*
 * 执行while语句
 */
static StatementResult execute_while_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;
    for (;;) {
        CRB_Value cond = crb_eval_expression(inter, env, statement->u.while_s.condition);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, statement->u.while_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
        if (!cond.u.boolean_value) {
            break;
        }
        result = crb_execute_statement_list(inter, env, statement->u.while_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.while_s.label, result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.while_s.label, result.type);
        }
    }
    return result;
}

/*
 * 执行for语句
 */
static StatementResult execute_for_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    if (statement->u.for_s.init) {
        crb_eval_expression(inter, env, statement->u.for_s.init);
    }
    for (;;) {
        if (statement->u.for_s.condition) {
            CRB_Value cond = crb_eval_expression(inter, env, statement->u.for_s.condition);
            if (cond.type != CRB_BOOLEAN_VALUE) {
                crb_runtime_error(inter, env, statement->u.for_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
            }
            DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value) {
                break;
            }
        }
        result = crb_execute_statement_list(inter, env, statement->u.for_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.for_s.label, result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.for_s.label, result.type);
        }

        if (statement->u.for_s.post) {
            crb_eval_expression(inter, env, statement->u.for_s.post);
        }
    }
    return result;
}

static CRB_Value* assign_to_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *name, CRB_Value *value) {
    CRB_Value* ret = crb_get_identifier_lvalue(inter, env, line_number, name);
    if (ret == NULL) {
        if (env != NULL) {
            ret = CRB_add_local_variable(inter, env, name, value, CRB_FALSE);
        } else {
            if (CRB_search_function(inter, name)) {
                crb_runtime_error(inter, env, line_number, FUNCTION_EXISTS_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", name, CRB_MESSAGE_ARGUMENT_END);
            }
            ret = CRB_add_global_variable(inter,  name, value, CRB_FALSE);
        }
    } else {
        *ret = *value;
    }
    return ret;
}

/*
 * 执行foreach语句
 */
static StatementResult execute_foreach_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    int stack_count = 0;

    StatementResult result;
    result.type = NORMAL_STATEMENT_RESULT;

    CRB_Value* collection = crb_eval_expression_peek(inter, env, statement->u.foreach_s.collection);
    ++stack_count;
    collection = CRB_peek_stack(inter, 0);

    CRB_Value iterator = CRB_call_method(inter, env, statement->line_number, collection->u.object, ITERATOR_METHOD_NAME, 0, NULL);
    CRB_push_value(inter, &iterator);
    ++stack_count;

    CRB_Value is_done;
    CRB_Value temp;
    temp.type = CRB_NULL_VALUE;
    CRB_Value* var = assign_to_variable(inter, env, statement->line_number, statement->u.foreach_s.variable, &temp);
    for (;;) {
        is_done = CRB_call_method(inter, env, statement->line_number, iterator.u.object, IS_DONE_METHOD_NAME, 0, NULL);
        if (is_done.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, statement->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (is_done.u.boolean_value) {
            break;
        }
        *var = CRB_call_method(inter, env, statement->line_number, iterator.u.object, CURRENT_ITEM_METHOD_NAME, 0, NULL);

        result = crb_execute_statement_list(inter, env, statement->u.for_s.block->statement_list);
        if (result.type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result.type == BREAK_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.for_s.label, result.type);
            break;
        } else if (result.type == CONTINUE_STATEMENT_RESULT) {
            result.type = compare_labels(result.u.label, statement->u.for_s.label, result.type);
        }

        CRB_call_method(inter, env, statement->line_number, iterator.u.object, NEXT_METHOD_NAME, 0, NULL);
    }
    CRB_shrink_stack(inter, stack_count);

    return result;
}

/*
 * 执行break语句
 */
static StatementResult execute_break_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = BREAK_STATEMENT_RESULT;
    result.u.label = statement->u.break_s.label;
    return result;
}

/*
 * 执行continue语句
 */
static StatementResult execute_continue_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    StatementResult result;
    result.type = CONTINUE_STATEMENT_RESULT;
    result.u.label = statement->u.continue_s.label;
    return result;
}

/*
 * 执行try语句
 */
static StatementResult execute_try_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    int stack_pointer_backup = crb_get_stack_pointer(inter);
    RecoveryEnvironment env_backup = inter->current_recovery_environment;
    StatementResult result;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        result = crb_execute_statement_list(inter, env, statement->u.try_s.try_block->statement_list);
    } else {
        crb_set_stack_pointer(inter, stack_pointer_backup);
        inter->current_recovery_environment = env_backup;

        if (statement->u.try_s.catch_block) {
            CRB_Value ex_value = inter->current_exception;
            CRB_push_value(inter, &ex_value);
            inter->current_exception.type = CRB_NULL_VALUE;

            assign_to_variable(inter, env, statement->line_number, statement->u.try_s.exception, &ex_value);

            result = crb_execute_statement_list(inter, env, statement->u.try_s.catch_block->statement_list);
            CRB_shrink_stack(inter, 1);
        }
    }
    inter->current_recovery_environment = env_backup;
    if (statement->u.try_s.finally_block) {
        crb_execute_statement_list(inter, env, statement->u.try_s.finally_block->statement_list);
    }
    if (!statement->u.try_s.catch_block && inter->current_exception.type != CRB_NULL_VALUE) {
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
    return result;
}

/*
 * 执行throw语句
 */
static StatementResult execute_throw_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement) {
    CRB_Value* ex_val = crb_eval_expression_peek(inter, env, statement->u.throw_s.exception);
    inter->current_exception = *ex_val;

    CRB_shrink_stack(inter, 1);

    longjmp(inter->current_recovery_environment.environment, LONGJMP_ARG);
}

/*
 * 执行语句
 */
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
        case RETURN_STATEMENT:
            result = execute_return_statement(inter, env, statement);
            break;
        case IF_STATEMENT:
            result = execute_if_statement(inter, env, statement);
            break;
        case WHILE_STATEMENT:
            result = execute_while_statement(inter, env, statement);
            break;
        case FOR_STATEMENT:
            result = execute_for_statement(inter, env, statement);
            break;
        case FOREACH_STATEMENT:
            result = execute_foreach_statement(inter, env, statement);
            break;
        case BREAK_STATEMENT:
            result = execute_break_statement(inter, env, statement);
            break;
        case CONTINUE_STATEMENT:
            result = execute_continue_statement(inter, env, statement);
            break;
        case TRY_STATEMENT:
            result = execute_try_statement(inter, env, statement);
            break;
        case THROW_STATEMENT:
            result = execute_throw_statement(inter, env, statement);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:   /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case...%d", statement->type));
    }

    return result;
}

inline StatementResult crb_execute_statement_list_with_pos(CRB_Interpreter *inter, CRB_LocalEnvironment *env, struct lnode* last_pos) {
    StatementResult ret;
    ret.type = NORMAL_STATEMENT_RESULT;
    for (struct lnode* node=last_pos->next; node!=NULL; node=node->next) {
        ret = execute_statement(inter, env, (Statement*)node->value.ptr_value);
        if (ret.type != NORMAL_STATEMENT_RESULT) {
            return ret;
        }
    }
    return ret;
}

StatementResult crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env, StatementList *list) {
    if (list == NULL) {
        StatementResult ret;
        ret.type = NORMAL_STATEMENT_RESULT;
        return ret;
    }
    return crb_execute_statement_list_with_pos(inter, env, llist_before_front_node(list));
}
