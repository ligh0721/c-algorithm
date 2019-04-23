//
// Created by t5w0rd on 19-4-20.
//

#include <math.h>
#include "tinterpreter.h"
#include "teval.h"
#include "terror.h"
#include "tstack.h"
#include "tmisc.h"
#include "theap.h"


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

/*
 * 在当前作用域下搜索被global关键字引用的全局变量
 */
static CRB_Value* search_global_variable_from_env(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *name, CRB_Boolean *is_final) {
    if (env == NULL) {
        return CRB_search_global_variable(inter, name, is_final);
    }

    GlobalVariableRef* global_ref = env->global_variable;
    NamedItemEntry key = {name};
    int ok;
    VALUE res = rbtree_get(global_ref, ptr_value(&key), &ok);
    if (ok) {
        return &((Variable*)res.ptr_value)->value;
    }
    return NULL;

//    for (GlobalVariableRef *pos; = env->global_variable; pos; pos = pos->next) {
//        if (!strcmp(pos->name, name)) {
//            return &pos->variable->value;
//        }
//    }
//
//    return NULL;
}

/*
 * 带有标识符的表达式求值
 */
static void eval_identifier_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    // 查找当前作用域内局部变量
    CRB_Value* vp = CRB_search_local_variable(env, expr->u.identifier, NULL);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    // 查找被global引用的全局变量
    CRB_Boolean is_final;
    vp = search_global_variable_from_env(inter, env, expr->u.identifier, &is_final);
    if (vp != NULL) {
        push_value(inter, vp);
        return;
    }

    // 查找定义的全局函数
    CRB_FunctionDefinition* func = CRB_search_function(inter, expr->u.identifier);
    if (func != NULL) {
        CRB_Value v;
        v.type = CRB_CLOSURE_VALUE;
        v.u.closure.function = func;
        v.u.closure.environment = NULL;
        push_value(inter, &v);
        return;
    }

    // 标识符不存在
    crb_runtime_error(inter, env, expr->line_number, VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.identifier, CRB_MESSAGE_ARGUMENT_END);
}

/*
 * 二元整数表达式求值
 */
static void eval_binary_int(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, int left, int right, CRB_Value *result, int line_number) {
    if (crb_is_math_operator(operator)) {
        result->type = CRB_INT_VALUE;
    } else if (crb_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_assert(crb_is_logical_operator(operator), ("operator..%d\n", operator));
        crb_runtime_error(inter, env, line_number, LOGICAL_OP_INTEGER_OPERAND_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
//        case REGEXP_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case COMMA_EXPRESSION:      /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_assert(0, ("bad case...%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.int_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.int_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.int_value = left * right;
            break;
        case DIV_EXPRESSION:
            if (right == 0) {
                crb_runtime_error(inter, env, line_number, DIVISION_BY_ZERO_ERR, CRB_MESSAGE_ARGUMENT_END);
            }
            result->u.int_value = left / right;
            break;
        case MOD_EXPRESSION:
            if (right == 0) {
                crb_runtime_error(inter, env, line_number, DIVISION_BY_ZERO_ERR, CRB_MESSAGE_ARGUMENT_END);
            }
            result->u.int_value = left % right;
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
        case LOGICAL_OR_EXPRESSION: /* FALLTHRU */
        case MINUS_EXPRESSION:      /* FALLTHRU */
        case LOGICAL_NOT_EXPRESSION:        /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case MEMBER_EXPRESSION:     /* FALLTHRU */
        case NULL_EXPRESSION:       /* FALLTHRU */
        case ARRAY_EXPRESSION:      /* FALLTHRU */
        case CLOSURE_EXPRESSION:    /* FALLTHRU */
        case INDEX_EXPRESSION:      /* FALLTHRU */
        case INCREMENT_EXPRESSION:  /* FALLTHRU */
        case DECREMENT_EXPRESSION:  /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case...%d", operator));
    }
}

/*
 * 二元浮点数表达式求值
 */
static void eval_binary_double(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, double left, double right, CRB_Value *result, int line_number) {
    if (crb_is_math_operator(operator)) {
        result->type = CRB_DOUBLE_VALUE;
    } else if (crb_is_compare_operator(operator)) {
        result->type = CRB_BOOLEAN_VALUE;
    } else {
        DBG_assert(crb_is_logical_operator(operator), ("operator..%d\n", operator));
        crb_runtime_error(inter, env, line_number, LOGICAL_OP_DOUBLE_OPERAND_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    switch (operator) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
//        case REGEXP_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION: /* FALLTHRU */
        case COMMA_EXPRESSION:      /* FALLTHRU */
        case ASSIGN_EXPRESSION:
            DBG_assert(0, ("bad case...%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.double_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.double_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.double_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.double_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.double_value = fmod(left, right);
            break;
        case EQ_EXPRESSION:
            result->u.boolean_value = left == right;
            break;
        case NE_EXPRESSION:
            result->u.boolean_value = left != right;
            break;
        case GT_EXPRESSION:
            result->u.boolean_value = left > right;
            break;
        case GE_EXPRESSION:
            result->u.boolean_value = left >= right;
            break;
        case LT_EXPRESSION:
            result->u.boolean_value = left < right;
            break;
        case LE_EXPRESSION:
            result->u.boolean_value = left <= right;
            break;
        case LOGICAL_AND_EXPRESSION:        /* FALLTHRU */
        case LOGICAL_OR_EXPRESSION:         /* FALLTHRU */
        case MINUS_EXPRESSION:              /* FALLTHRU */
        case LOGICAL_NOT_EXPRESSION:        /* FALLTHRU */
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case MEMBER_EXPRESSION:     /* FALLTHRU */
        case NULL_EXPRESSION:               /* FALLTHRU */
        case ARRAY_EXPRESSION:      /* FALLTHRU */
        case CLOSURE_EXPRESSION:    /* FALLTHRU */
        case INDEX_EXPRESSION:      /* FALLTHRU */
        case INCREMENT_EXPRESSION:
        case DECREMENT_EXPRESSION:
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_assert(0, ("bad default...%d", operator));
    }
}

/*
 * 二元数字表达式求值
 */
void eval_binary_numeric(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, CRB_Value *left_val, CRB_Value *right_val, CRB_Value *result, int line_number) {
    if (left_val->type == CRB_INT_VALUE && right_val->type == CRB_INT_VALUE) {
        eval_binary_int(inter, env, operator, left_val->u.int_value, right_val->u.int_value, result, line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, env, operator, left_val->u.double_value, right_val->u.double_value, result, line_number);
    } else if (left_val->type == CRB_INT_VALUE && right_val->type == CRB_DOUBLE_VALUE) {
        eval_binary_double(inter, env, operator, (double)left_val->u.int_value, right_val->u.double_value, result, line_number);
    } else if (left_val->type == CRB_DOUBLE_VALUE && right_val->type == CRB_INT_VALUE) {
        eval_binary_double(inter, env, operator, left_val->u.double_value, (double)right_val->u.int_value, result, line_number);
    }
}

/*
 * 连接字符串
 */
void chain_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *left, CRB_Value *right, CRB_Value *result) {
    CRB_Char* right_str = CRB_value_to_string(inter, env, line_number, right);
    CRB_Object* right_obj = crb_create_crowbar_string_i(inter, right_str);

    result->type = CRB_STRING_VALUE;
//    int len = wstring_len(left->u.object->u.string.string) + wstring_len(right_obj->u.string.string);
    int len = CRB_wcslen(left->u.object->u.string.string) + CRB_wcslen(right_obj->u.string.string);
    CRB_Char* str = MEM_malloc(sizeof(CRB_Char) * (len + 1));
    CRB_wcscpy(str, left->u.object->u.string.string);
    CRB_wcscat(str, right_obj->u.string.string);
    result->u.object = crb_create_crowbar_string_i(inter, str);
}

/*
 * 赋值
 */
static void do_assign(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Value *src, CRB_Value *dest, AssignmentOperator operator, int line_number) {
    ExpressionType expr_type;
    CRB_Value result;

    if (operator == NORMAL_ASSIGN) {
        *dest = *src;
    } else {
        switch (operator) {
            case NORMAL_ASSIGN:
                DBG_panic(("NORMAL_ASSIGN.\n"));
            case ADD_ASSIGN:
                expr_type = ADD_EXPRESSION;
                break;
            case SUB_ASSIGN:
                expr_type = SUB_EXPRESSION;
                break;
            case MUL_ASSIGN:
                expr_type = MUL_EXPRESSION;
                break;
            case DIV_ASSIGN:
                expr_type = DIV_EXPRESSION;
                break;
            case MOD_ASSIGN:
                expr_type = MOD_EXPRESSION;
                break;
            default:
                DBG_panic(("bad default.\n"));
        }
        if (dest->type == CRB_STRING_VALUE && expr_type == ADD_EXPRESSION) {
            chain_string(inter, env, line_number, dest, src, &result);
        } else {
            eval_binary_numeric(inter, env, expr_type, dest, src, &result, line_number);
        }
        *dest = result;
    }
}

static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

/*
 * 关联数组赋值
 * expr: left.right = rvalue
 * src: lvalue
 */
static void assign_to_member(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr, CRB_Value *src) {
    Expression *left = expr->u.assign_expression.left;
    CRB_Boolean is_final;
    eval_expression(inter, env, left->u.member_expression.expression);
    CRB_Value* assoc = peek_stack(inter, 0);
    if (assoc->type != CRB_ASSOC_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, NOT_OBJECT_MEMBER_ASSIGN_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Value* dest = CRB_search_assoc_member(assoc->u.object, left->u.member_expression.member_name, &is_final);
    if (dest == NULL) {
        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
            crb_runtime_error(inter, env, expr->line_number, NO_SUCH_MEMBER_ERR, CRB_STRING_MESSAGE_ARGUMENT, "member_name", left->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_add_assoc_member(inter, assoc->u.object, left->u.member_expression.member_name, src, expr->u.assign_expression.is_final);
    } else {
        if (is_final) {
            crb_runtime_error(inter, env, expr->line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
        }
        do_assign(inter, env, src, dest, expr->u.assign_expression.operator, expr->line_number);
    }
    pop_value(inter);
}

/*
 * 获取标识符左值
 */
CRB_Value* crb_get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, char *identifier) {
    CRB_Boolean is_final = CRB_FALSE;
    CRB_Value* left = CRB_search_local_variable(env, identifier, &is_final);
    if (left == NULL) {
        left = search_global_variable_from_env(inter, env, identifier, &is_final);
    }
    if (is_final) {
        crb_runtime_error(inter, env, line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", identifier, CRB_MESSAGE_ARGUMENT_END);
    }
    return left;
}

/*
 * 获取数组元素左值
 */
static CRB_Value* get_array_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.index_expression.array);
    eval_expression(inter, env, expr->u.index_expression.index);
    CRB_Value index = pop_value(inter);
    CRB_Value array = pop_value(inter);

    if (array.type != CRB_ARRAY_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_ARRAY_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Value_ARRAY* arr = array.u.object->u.array.array;
    long len = CRB_Value_array_len(arr);
    CRB_Value* data = CRB_Value_array_data(arr);
    if (index.u.int_value < 0 || index.u.int_value >= len) {
        crb_runtime_error(inter, env, expr->line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", len, CRB_INT_MESSAGE_ARGUMENT, "index", index.u.int_value, CRB_MESSAGE_ARGUMENT_END);
    }
    return data + index.u.int_value;
}
/*
 * 获取关联数组成员左值
 */
static CRB_Value* get_member_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.member_expression.expression);
    CRB_Value assoc = pop_value(inter);

    if (assoc.type != CRB_ASSOC_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, NOT_OBJECT_MEMBER_UPDATE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Boolean is_final = CRB_FALSE;
    CRB_Value* dest = CRB_search_assoc_member(assoc.u.object, expr->u.member_expression.member_name, &is_final);
    if (is_final) {
        crb_runtime_error(inter, env, expr->line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
    }
    return dest;
}

/*
 * 获取左值
 */
static CRB_Value* get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    CRB_Value* dest = NULL;
    if (expr->type == IDENTIFIER_EXPRESSION) {
        dest = crb_get_identifier_lvalue(inter, env, expr->line_number, expr->u.identifier);
    } else if (expr->type == INDEX_EXPRESSION) {
        dest = get_array_element_lvalue(inter, env, expr);
    } else if (expr->type == MEMBER_EXPRESSION) {
        dest = get_member_lvalue(inter, env, expr);
    } else {
        crb_runtime_error(inter, env, expr->line_number, NOT_LVALUE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    return dest;
}

/*
 * 赋值表达式求值
 */
static void eval_assign_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    Expression* left = expr->u.assign_expression.left;

    // 计算右值
    eval_expression(inter, env, expr->u.assign_expression.operand);
    CRB_Value* src = peek_stack(inter, 0);

    if (left->type == MEMBER_EXPRESSION) {
        assign_to_member(inter, env, expr, src);
        return;
    }

    CRB_Value* dest = get_lvalue(inter, env, left);
    if (left->type == IDENTIFIER_EXPRESSION && dest == NULL) {
        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
            crb_runtime_error(inter, env, expr->line_number, VARIABLE_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
        }
        if (env != NULL) {
            CRB_add_local_variable(inter, env, left->u.identifier, src, expr->u.assign_expression.is_final);
        } else {
            if (CRB_search_function(inter, left->u.identifier)) {
                crb_runtime_error(inter, env, expr->line_number, FUNCTION_EXISTS_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
            }
            CRB_add_global_variable(inter, left->u.identifier, src, expr->u.assign_expression.is_final);
        }
    } else {
        DBG_assert(dest != NULL, ("dest == NULL.\n"));
        do_assign(inter, env, src, dest, expr->u.assign_expression.operator, expr->line_number);
    }
}

static void eval_string_expression(CRB_Interpreter *inter, CRB_Char *string_value) {
    CRB_Value v;
    v.type = CRB_STRING_VALUE;
    v.u.object = crb_literal_to_crb_string_i(inter, string_value);
    push_value(inter, &v);
}

/*
 * 表达式求值，入栈
 */
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
        // TODO:
//        case COMMA_EXPRESSION:
//            eval_comma_expression(inter, env, expr);
//            break;
        case ASSIGN_EXPRESSION:
            eval_assign_expression(inter, env, expr);
            break;
//        case ADD_EXPRESSION:        /* FALLTHRU */
//        case SUB_EXPRESSION:        /* FALLTHRU */
//        case MUL_EXPRESSION:        /* FALLTHRU */
//        case DIV_EXPRESSION:        /* FALLTHRU */
//        case MOD_EXPRESSION:        /* FALLTHRU */
//        case EQ_EXPRESSION: /* FALLTHRU */
//        case NE_EXPRESSION: /* FALLTHRU */
//        case GT_EXPRESSION: /* FALLTHRU */
//        case GE_EXPRESSION: /* FALLTHRU */
//        case LT_EXPRESSION: /* FALLTHRU */
//        case LE_EXPRESSION:
//            eval_binary_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
//            break;
//        case LOGICAL_AND_EXPRESSION:/* FALLTHRU */
//        case LOGICAL_OR_EXPRESSION: /* FALLTHRU */
//            eval_logical_and_or_expression(inter, env, expr->type, expr->u.binary_expression.left, expr->u.binary_expression.right);
//            break;
//        case MINUS_EXPRESSION:
//            eval_minus_expression(inter, env, expr->u.minus_expression);
//            break;
//        case LOGICAL_NOT_EXPRESSION:
//            eval_logical_not_expression(inter, env, expr->u.minus_expression);
//            break;
//        case FUNCTION_CALL_EXPRESSION:
//            eval_function_call_expression(inter, env, expr);
//            break;
//        case MEMBER_EXPRESSION:
//            eval_member_expression(inter, env, expr);
//            break;
//        case NULL_EXPRESSION:
//            eval_null_expression(inter);
//            break;
//        case ARRAY_EXPRESSION:
//            eval_array_expression(inter, env, expr->u.array_literal);
//            break;
//        case CLOSURE_EXPRESSION:
//            eval_closure_expression(inter, env, expr);
//            break;
//        case INDEX_EXPRESSION:
//            eval_index_expression(inter, env, expr);
//            break;
//        case INCREMENT_EXPRESSION:  /* FALLTHRU */
//        case DECREMENT_EXPRESSION:
//            eval_inc_dec_expression(inter, env, expr);
//            break;
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_assert(0, ("bad case. type..%d\n", expr->type));
    }
}

CRB_Value crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr);
    return pop_value(inter);
}
