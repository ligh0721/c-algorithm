//
// Created by t5w0rd on 19-4-20.
//

#include "teval.h"
#include "terror.h"


CRB_Value CRB_call_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *func, int arg_count, CRB_Value *args) {
    CRB_Value ret = {};
    // TODO:
    return ret;
}

static void eval_boolean_expression(CRB_Interpreter *inter, CRB_Boolean boolean_value) {
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    push_value(inter, &v);
}

static void eval_int_expression(CRB_Interpreter *inter, int int_value) {
    CRB_Value v;
    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;
    push_value(inter, &v);
}

static void eval_double_expression(CRB_Interpreter *inter, double double_value) {
    CRB_Value v;
    v.type = CRB_DOUBLE_VALUE;
    v.u.double_value = double_value;
    push_value(inter, &v);
}

static void
eval_identifier_expression(CRB_Interpreter *inter,
                           CRB_LocalEnvironment *env, Expression *expr)
{
    CRB_Value *vp;
    CRB_FunctionDefinition *func;
    CRB_Boolean is_final; /* dummy */

    vp = CRB_search_local_variable(env, expr->u.identifier);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    vp = search_global_variable_from_env(inter, env, expr->u.identifier,
                                         &is_final);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    func = CRB_search_function(inter, expr->u.identifier);
    if (func != NULL) {
        CRB_Value       v;
        v.type = CRB_CLOSURE_VALUE;
        v.u.closure.function = func;
        v.u.closure.environment = NULL;
        push_value(inter, &v);
        return;
    }

    crb_runtime_error(inter, env, expr->line_number, VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.identifier, CRB_MESSAGE_ARGUMENT_END);
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    switch (expr->type) {
        case BOOLEAN_EXPRESSION:
            eval_boolean_expression(inter, expr->u.boolean_value);
            break;
        case INT_EXPRESSION:
            eval_int_expression(inter, expr->u.int_value);
            break;
        case DOUBLE_EXPRESSION:
            eval_double_expression(inter, expr->u.double_value);
            break;
        case STRING_EXPRESSION:
            eval_string_expression(inter, expr->u.string_value);
            break;
//        case REGEXP_EXPRESSION:
//            eval_regexp_expression(inter, expr->u.regexp_value);
//            break;
        case IDENTIFIER_EXPRESSION:
            eval_identifier_expression(inter, env, expr);
            break;
        case COMMA_EXPRESSION:
            eval_comma_expression(inter, env, expr);
            break;
        case ASSIGN_EXPRESSION:
            eval_assign_expression(inter, env, expr);
            break;
        case ADD_EXPRESSION:        /* FALLTHRU */
        case SUB_EXPRESSION:        /* FALLTHRU */
        case MUL_EXPRESSION:        /* FALLTHRU */
        case DIV_EXPRESSION:        /* FALLTHRU */
        case MOD_EXPRESSION:        /* FALLTHRU */
        case EQ_EXPRESSION: /* FALLTHRU */
        case NE_EXPRESSION: /* FALLTHRU */
        case GT_EXPRESSION: /* FALLTHRU */
        case GE_EXPRESSION: /* FALLTHRU */
        case LT_EXPRESSION: /* FALLTHRU */
        case LE_EXPRESSION:
            eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
        case LOGICAL_OR_EXPRESSION: /* FALLTHRU */
            eval_logical_and_or_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
            break;
        case MINUS_EXPRESSION:
            eval_minus_expression(inter, env, expr->u.minus_expression);
            break;
        case LOGICAL_NOT_EXPRESSION:
            eval_logical_not_expression(inter, env, expr->u.minus_expression);
            break;
        case FUNCTION_CALL_EXPRESSION:
            eval_function_call_expression(inter, env, expr);
            break;
        case MEMBER_EXPRESSION:
            eval_member_expression(inter, env, expr);
            break;
        case NULL_EXPRESSION:
            eval_null_expression(inter);
            break;
        case ARRAY_EXPRESSION:
            eval_array_expression(inter, env, expr->u.array_literal);
            break;
        case CLOSURE_EXPRESSION:
            eval_closure_expression(inter, env, expr);
            break;
        case INDEX_EXPRESSION:
            eval_index_expression(inter, env, expr);
            break;
        case INCREMENT_EXPRESSION:  /* FALLTHRU */
        case DECREMENT_EXPRESSION:
            eval_inc_dec_expression(inter, env, expr);
            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case. type..%d\n", expr->type));
    }
}

CRB_Value crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr);
    return pop_value(inter);
}