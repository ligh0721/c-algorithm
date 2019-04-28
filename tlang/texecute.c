//
// Created by t5w0rd on 19-4-20.
//

#include "tinterpreter.h"
#include "texecute.h"
#include "terror.h"
#include "tmisc.h"
#include "teval.h"
#include "heap.h"


static void execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result);

/*
 * 执行表达式语句
 */
static void execute_expression_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    CRB_Value dummy;
    crb_eval_expression(inter, env, statement->u.expression_s, &dummy);
}

/*
 * 执行全局引用语句
 */
static void execute_global_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    if (env == NULL) {
        crb_runtime_error(inter, env, statement->line_number, GLOBAL_STATEMENT_IN_TOPLEVEL_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    GlobalVariableRef* global_ref = env->global_var_refs;
    IdentifierList* global_identifiers = statement->u.global_s.identifier_list;
    for (struct lnode* node=global_identifiers?llist_front_node(global_identifiers):NULL; node!=NULL; node=node->next) {
        char* identifier_name = (char*)node->value.ptr_value;
        RBNODE* parent;
        RBNODE** where;
        if (global_ref != NULL) {
            NamedItemEntry key = {identifier_name};
            where = rbtree_fast_get(global_ref, ptr_value(&key), &parent);
            if (!rbtree_node_not_found(global_ref, where)) {
                continue;
            }
        }

        Variable* variable = crb_search_global_variable(inter, identifier_name);
        if (variable == NULL) {
            crb_runtime_error(inter, env, statement->line_number, GLOBAL_VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", identifier_name, CRB_MESSAGE_ARGUMENT_END);
        }
        if (global_ref != NULL) {
            RBNODE* var_node = rbtree_open_node(global_ref, ptr_value(variable), parent);
            rbtree_fast_set(global_ref, where, var_node);
        } else {
            global_ref = env->global_var_refs = open_rbtree(_crb_asc_order_named_item);
            rbtree_set(global_ref, ptr_value(variable));
        }
    }
}

/*
 * 执行return语句
 */
static void execute_return_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = RETURN_STATEMENT_RESULT;
    if (statement->u.return_s.return_value) {
        crb_eval_expression(inter, env, statement->u.return_s.return_value, &result->u.return_value);
    } else {
        result->u.return_value.type = CRB_NULL_VALUE;
    }
}

/*
 * 执行elif部分
 */
static void execute_elif(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ElifList *elif_list, CRB_Boolean *executed, StatementResult* result) {
    *executed = CRB_FALSE;
    result->type = NORMAL_STATEMENT_RESULT;
    CRB_Value cond;
    for (struct lnode* node=elif_list?llist_front_node(elif_list):NULL; node!=NULL; node=node->next) {
        Elif* elif = (Elif*)node->value.ptr_value;
        crb_eval_expression(inter, env, elif->condition, &cond);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, elif->condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (cond.u.boolean_value) {
            crb_execute_statement_list(inter, env, elif->block->statement_list, result);
            *executed = CRB_TRUE;
            if (result->type != NORMAL_STATEMENT_RESULT) {
                return;
            }
        }
    }
}

/*
 * 执行if语句
 */
static void execute_if_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    CRB_Value cond;
    crb_eval_expression(inter, env, statement->u.if_s.condition, &cond);
    if (cond.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, statement->u.if_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));

    if (cond.u.boolean_value) {
        crb_execute_statement_list(inter, env, statement->u.if_s.then_block->statement_list, result);
    } else {
        CRB_Boolean elif_executed;
        execute_elif(inter, env, statement->u.if_s.elif_list, &elif_executed, result);
        if (result->type != NORMAL_STATEMENT_RESULT) {
            return;
        }
        if (!elif_executed && statement->u.if_s.else_block) {
            crb_execute_statement_list(inter, env, statement->u.if_s.else_block->statement_list, result);
        }
    }
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
static void execute_while_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    CRB_Value cond;
    for (;;) {
        crb_eval_expression(inter, env, statement->u.while_s.condition, &cond);
        if (cond.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, statement->u.while_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
        if (!cond.u.boolean_value) {
            break;
        }
        crb_execute_statement_list(inter, env, statement->u.while_s.block->statement_list, result);
        if (result->type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result->type == BREAK_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.while_s.label, result->type);
            break;
        } else if (result->type == CONTINUE_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.while_s.label, result->type);
        }
    }
}

/*
 * 执行for语句
 */
static void execute_for_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    CRB_Value dummy;
    if (statement->u.for_s.init) {
        crb_eval_expression(inter, env, statement->u.for_s.init, &dummy);
    }

    CRB_Value cond;
    for (;;) {
        if (statement->u.for_s.condition) {
            crb_eval_expression(inter, env, statement->u.for_s.condition, &cond);
            if (cond.type != CRB_BOOLEAN_VALUE) {
                crb_runtime_error(inter, env, statement->u.for_s.condition->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
            }
            DBG_assert(cond.type == CRB_BOOLEAN_VALUE, ("cond.type..%d", cond.type));
            if (!cond.u.boolean_value) {
                break;
            }
        }
        crb_execute_statement_list(inter, env, statement->u.for_s.block->statement_list, result);
        if (result->type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result->type == BREAK_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.for_s.label, result->type);
            break;
        } else if (result->type == CONTINUE_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.for_s.label, result->type);
        }

        if (statement->u.for_s.post) {
            crb_eval_expression(inter, env, statement->u.for_s.post, &dummy);
        }
    }
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
static void execute_foreach_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    int stack_count = 0;

    result->type = NORMAL_STATEMENT_RESULT;

    CRB_Value* collection = crb_eval_expression_peek(inter, env, statement->u.foreach_s.collection);
    ++stack_count;
    collection = CRB_peek_stack(inter, 0);

    CRB_Value iterator;
    CRB_call_method(inter, env, statement->line_number, collection->u.object, ITERATOR_METHOD_NAME, 0, NULL, &iterator);
    CRB_push_value(inter, &iterator);
    ++stack_count;

    CRB_Value is_done;
    CRB_Value temp;
    temp.type = CRB_NULL_VALUE;
    CRB_Value* var = assign_to_variable(inter, env, statement->line_number, statement->u.foreach_s.variable, &temp);
    CRB_Value dummy;
    for (;;) {
        CRB_call_method(inter, env, statement->line_number, iterator.u.object, IS_DONE_METHOD_NAME, 0, NULL, &is_done);
        if (is_done.type != CRB_BOOLEAN_VALUE) {
            crb_runtime_error(inter, env, statement->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (is_done.u.boolean_value) {
            break;
        }
        CRB_call_method(inter, env, statement->line_number, iterator.u.object, CURRENT_ITEM_METHOD_NAME, 0, NULL, var);

        crb_execute_statement_list(inter, env, statement->u.for_s.block->statement_list, result);
        if (result->type == RETURN_STATEMENT_RESULT) {
            break;
        } else if (result->type == BREAK_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.for_s.label, result->type);
            break;
        } else if (result->type == CONTINUE_STATEMENT_RESULT) {
            result->type = compare_labels(result->u.label, statement->u.for_s.label, result->type);
        }

        CRB_call_method(inter, env, statement->line_number, iterator.u.object, NEXT_METHOD_NAME, 0, NULL, &dummy);
    }
    CRB_shrink_stack(inter, stack_count);
}

/*
 * 执行break语句
 */
static void execute_break_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = BREAK_STATEMENT_RESULT;
    result->u.label = statement->u.break_s.label;
}

/*
 * 执行continue语句
 */
static void execute_continue_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = CONTINUE_STATEMENT_RESULT;
    result->u.label = statement->u.continue_s.label;
}

/*
 * 执行try语句
 */
static void execute_try_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    int stack_pointer_backup = crb_get_stack_pointer(inter);
    RecoveryEnvironment env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        crb_execute_statement_list(inter, env, statement->u.try_s.try_block->statement_list, result);
    } else {
        crb_set_stack_pointer(inter, stack_pointer_backup);
        inter->current_recovery_environment = env_backup;

        if (statement->u.try_s.catch_block) {
            CRB_Value ex_value = inter->current_exception;
            CRB_push_value(inter, &ex_value);
            inter->current_exception.type = CRB_NULL_VALUE;

            assign_to_variable(inter, env, statement->line_number, statement->u.try_s.exception, &ex_value);

            crb_execute_statement_list(inter, env, statement->u.try_s.catch_block->statement_list, result);
            CRB_shrink_stack(inter, 1);
        }
    }
    inter->current_recovery_environment = env_backup;
    if (statement->u.try_s.finally_block) {
        StatementResult dummy;
        crb_execute_statement_list(inter, env, statement->u.try_s.finally_block->statement_list, &dummy);
    }
    if (!statement->u.try_s.catch_block && inter->current_exception.type != CRB_NULL_VALUE) {
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
}

/*
 * 执行throw语句
 */
static void execute_throw_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    CRB_Value* ex_val = crb_eval_expression_peek(inter, env, statement->u.throw_s.exception);
    inter->current_exception = *ex_val;
    CRB_shrink_stack(inter, 1);
    longjmp(inter->current_recovery_environment.environment, LONGJMP_ARG);
}

/*
 * 执行import语句
 */
static void execute_import_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    int len = CRB_wcstombs_len(statement->u.import_s.name->u.string_value);
    char* name = crb_execute_malloc(inter, len+1);
    CRB_wcstombs(statement->u.import_s.name->u.string_value, name);
    CRB_import_model(inter, name);
}

/*
 * 执行语句
 */
static void execute_statement(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Statement *statement, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    switch (statement->type) {
        case EXPRESSION_STATEMENT:
            execute_expression_statement(inter, env, statement, result);
            break;
        case GLOBAL_STATEMENT:
            execute_global_statement(inter, env, statement, result);
            break;
        case RETURN_STATEMENT:
            execute_return_statement(inter, env, statement, result);
            break;
        case IF_STATEMENT:
            execute_if_statement(inter, env, statement, result);
            break;
        case WHILE_STATEMENT:
            execute_while_statement(inter, env, statement, result);
            break;
        case FOR_STATEMENT:
            execute_for_statement(inter, env, statement, result);
            break;
        case FOREACH_STATEMENT:
            execute_foreach_statement(inter, env, statement, result);
            break;
        case BREAK_STATEMENT:
            execute_break_statement(inter, env, statement, result);
            break;
        case CONTINUE_STATEMENT:
            execute_continue_statement(inter, env, statement, result);
            break;
        case TRY_STATEMENT:
            execute_try_statement(inter, env, statement, result);
            break;
        case THROW_STATEMENT:
            execute_throw_statement(inter, env, statement, result);
            break;
        case IMPORT_STATEMENT:
            execute_import_statement(inter, env, statement, result);
            break;
        case STATEMENT_TYPE_COUNT_PLUS_1:   /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case...%d", statement->type));
    }
}

inline void crb_execute_statement_list_with_pos(CRB_Interpreter *inter, CRB_LocalEnvironment *env, struct lnode* last_pos, StatementResult* result) {
    result->type = NORMAL_STATEMENT_RESULT;
    for (struct lnode* node=last_pos->next; node!=NULL; node=node->next) {
        execute_statement(inter, env, (Statement*)node->value.ptr_value, result);
        if (result->type != NORMAL_STATEMENT_RESULT) {
            return;
        }
    }
}

void crb_execute_statement_list(CRB_Interpreter *inter, CRB_LocalEnvironment *env, StatementList *list, StatementResult* result) {
    if (list == NULL) {
        result->type = NORMAL_STATEMENT_RESULT;
        return;
    }
    crb_execute_statement_list_with_pos(inter, env, llist_before_front_node(list), result);
}
