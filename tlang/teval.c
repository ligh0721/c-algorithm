//
// Created by t5w0rd on 19-4-20.
//

#include <math.h>
#include <include/tlang.h>
#include "tinterpreter.h"
#include "teval.h"
#include "terror.h"
#include "tmisc.h"
#include "theap.h"
#include "texecute.h"
#include "tnative.h"


// stack operation
static inline void push_value(CRB_Interpreter *inter, CRB_Value *value) {
    DBG_assert(inter->stack.stack_pointer <= inter->stack.stack_alloc_size, ("stack_pointer..%d, stack_alloc_size..%d\n", inter->stack.stack_pointer, inter->stack.stack_alloc_size));

    if (inter->stack.stack_pointer == inter->stack.stack_alloc_size) {
        inter->stack.stack_alloc_size += STACK_ALLOC_SIZE;
        inter->stack.stack = MEM_realloc(inter->stack.stack, sizeof(CRB_Value) * inter->stack.stack_alloc_size);
    }
    inter->stack.stack[inter->stack.stack_pointer] = *value;
    inter->stack.stack_pointer++;
}

void CRB_push_value(CRB_Interpreter *inter, CRB_Value *value) {
    push_value(inter, value);
}

static inline void pop_value(CRB_Interpreter *inter, CRB_Value *popped) {
    DBG_assert(popped != NULL, ("popped cannot be NULL, use shrink_stack() instead\n"));
    *popped = inter->stack.stack[inter->stack.stack_pointer-1];
    inter->stack.stack_pointer--;
}

void CRB_pop_value(CRB_Interpreter *inter, CRB_Value* popped) {
    pop_value(inter, popped);
}

static inline CRB_Value* peek_stack(CRB_Interpreter *inter, int index) {
    return &inter->stack.stack[inter->stack.stack_pointer-index-1];
}

CRB_Value* CRB_peek_stack(CRB_Interpreter *inter, int index) {
    return peek_stack(inter, index);
}

inline void crb_set_stack_pointer(CRB_Interpreter *inter, int stack_pointer) {
    inter->stack.stack_pointer = stack_pointer;
}

inline int crb_get_stack_pointer(CRB_Interpreter *inter) {
    return inter->stack.stack_pointer;
}

static inline void shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    inter->stack.stack_pointer -= shrink_size;
}

inline void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size) {
    shrink_stack(inter, shrink_size);
}

/*
 * 分配作用域
 */
static CRB_LocalEnvironment* alloc_local_environment(CRB_Interpreter *inter, CRB_Module* module, const char *func_name, int caller_line_number, CRB_Object *closure_scope_chain) {
    CRB_LocalEnvironment* ret = MEM_malloc(sizeof(CRB_LocalEnvironment));
    ret->next = inter->top_environment;
    inter->top_environment = ret;

    ret->module = module;
    ret->current_function_name = func_name;
    ret->caller_line_number = caller_line_number;

    // 接下来的crb_create_xxx()包含alloc_object操作，可能会触发GC，GC会扫描ref_in_native_method和scope_chain，所以这里提前设为NULL
    ret->ref_in_native_method = NULL; /* to stop marking by GC */
    ret->scope_chain = NULL; /* to stop marking by GC */

    ret->scope_chain = crb_create_scope_chain(inter);  // 每个函数的执行环境都有自己的作用域链的起始指针
    ret->scope_chain->u.scope_chain.frame = crb_create_assoc_i(inter);
    ret->scope_chain->u.scope_chain.next = closure_scope_chain;
    ret->global_var_refs = NULL;
    return ret;
}

// 表达式求值前置声明
static void eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr);

/*
 * 从当前作用域中的全局引用表中搜索变量
 */
static CRB_Value* search_global_variable_from_env(CRB_Interpreter *inter, CRB_LocalEnvironment *env, const char *name, CRB_Boolean *is_final) {
    if (env == NULL) {
        CRB_Value* ret = CRB_search_global_variable(inter, inter->current_module, name, is_final);
        if (ret != NULL) {
            return ret;
        }
        return CRB_search_global_variable(inter, NULL, name, is_final);
    }

    GlobalVariableRef* global_ref = env->global_var_refs;
    if (global_ref == NULL) {
        return NULL;
    }
    NamedItemEntry key = {name};
    int ok;
    VALUE res = rbtree_get(global_ref, ptr_value(&key), &ok);
    if (ok) {
        return &((Variable*)res.ptr_value)->value;
    }
    return NULL;
}

/*
 * NULL字面量
 */
static void eval_null_expression(CRB_Interpreter *inter) {
    push_value(inter, &CRB_Null_Value);
}

/*
 * 布尔类型字面量
 */
static void eval_boolean_expression(CRB_Interpreter *inter, CRB_Boolean boolean_value) {
    CRB_Value v;
    v.type = CRB_BOOLEAN_VALUE;
    v.u.boolean_value = boolean_value;
    push_value(inter, &v);
}

/*
 * 整数类型字面量
 */
static void eval_int_expression(CRB_Interpreter *inter, long int_value) {
    CRB_Value v;
    v.type = CRB_INT_VALUE;
    v.u.int_value = int_value;
    push_value(inter, &v);
}

/*
 * 浮点类型字面量
 */
static void eval_double_expression(CRB_Interpreter *inter, double float_value) {
    CRB_Value v;
    v.type = CRB_FLOAT_VALUE;
    v.u.float_value = float_value;
    push_value(inter, &v);
}

/*
 * 数组字面量
 */
static void eval_array_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionList *list) {
//    int         i;

//    int size = 0;
//    for (ExpressionList* pos = list; pos; pos = pos->next) {
//        size++;
//    }
    long len;
    struct lnode* node;
    if (list != NULL) {
        len = llist_len(list);
        node = llist_front_node(list);
    } else {
        len = 0;
        node = NULL;
    }

    CRB_Value v;
    v.type = CRB_ARRAY_VALUE;
    v.u.object = crb_create_array_i(inter, len);
    push_value(inter, &v);

    CRB_Value* data = CRB_Value_slice_data(v.u.object->u.array.array);
    for (; node!=NULL; node=node->next) {
        eval_expression(inter, env, (Expression*)node->value.ptr_value);
        pop_value(inter, data++);
    }

//    for (ExpressionList* pos = list, i = 0; pos; pos = pos->next, i++) {
//        eval_expression(inter, env, pos->expression);
//        v.u.object->u.array.array[i] = pop_value(inter);
//    }
}

/*
 * 对象(关联数组)字面量
 */
static void eval_assoc_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, AssocExpressionList *list) {
    struct lnode* node;
    if (list != NULL) {
        node = llist_front_node(list);
    } else {
        node = NULL;
    }

    CRB_Value v;
    v.type = CRB_ASSOC_VALUE;
    v.u.object = crb_create_assoc_i(inter);
    push_value(inter, &v);

    for (; node!=NULL; node=node->next) {
        AssocExpression* assoc_expr = (AssocExpression*)node->value.ptr_value;
        eval_expression(inter, env, assoc_expr->expression);
        CRB_Value* value = peek_stack(inter, 0);
        CRB_add_assoc_member(inter, v.u.object, assoc_expr->member_name, value, assoc_expr->is_final);
        shrink_stack(inter, 1);
    }
}

/*
 * 闭包字面量
 */
static void eval_closure_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    CRB_Value   result;
    result.type = CRB_CLOSURE_VALUE;
    result.u.closure.function_definition = expr->u.closure.function_definition;
    if (env) {
        result.u.closure.scope_chain = env->scope_chain;
    } else {
        result.u.closure.scope_chain = NULL;
    }
    push_value(inter, &result);
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

    // 查找定义的全局函数，找到后分配一个函数值类型压栈返回
    CRB_FunctionDefinition* fd = CRB_search_function(inter, CRB_env_module(inter, env), expr->u.identifier);
    if (fd == NULL) {
        fd = CRB_search_function(inter, NULL, expr->u.identifier);
    }
    if (fd != NULL) {
        CRB_Value func;
        CRB_create_closure(NULL, fd, &func);
        push_value(inter, &func);
        return;
    }

    // 查找已加的载模块，找到后分配一个模块值类型，压栈返回
    CRB_Module* module = CRB_search_module(inter, expr->u.identifier);
    if (module != NULL) {
        CRB_Value v;
        v.type = CRB_MODULE_VALUE;
        v.u.module = module;
        push_value(inter, &v);
        return;
    }

    // 标识符不存在
    crb_runtime_error(inter, env, expr->line_number, IDENTIFIER_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.identifier, CRB_MESSAGE_ARGUMENT_END);
}

/*
 * 二元整数表达式求值
 */
static void eval_binary_int(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, long left, long right, CRB_Value *result, int line_number) {
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
        case CONCAT_STRING_EXPRESSION:
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
        case ASSOC_EXPRESSION:      /* FALLTHRU */
        case CLOSURE_EXPRESSION:    /* FALLTHRU */
        case INDEX_EXPRESSION:      /* FALLTHRU */
        case SLICE_EXPRESSION:      /* FALLTHRU */
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
        result->type = CRB_FLOAT_VALUE;
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
        case CONCAT_STRING_EXPRESSION:
            DBG_assert(0, ("bad case...%d", operator));
            break;
        case ADD_EXPRESSION:
            result->u.float_value = left + right;
            break;
        case SUB_EXPRESSION:
            result->u.float_value = left - right;
            break;
        case MUL_EXPRESSION:
            result->u.float_value = left * right;
            break;
        case DIV_EXPRESSION:
            result->u.float_value = left / right;
            break;
        case MOD_EXPRESSION:
            result->u.float_value = fmod(left, right);
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
    } else if (left_val->type == CRB_FLOAT_VALUE && right_val->type == CRB_FLOAT_VALUE) {
        eval_binary_double(inter, env, operator, left_val->u.float_value, right_val->u.float_value, result, line_number);
    } else if (left_val->type == CRB_INT_VALUE && right_val->type == CRB_FLOAT_VALUE) {
        eval_binary_double(inter, env, operator, (double)left_val->u.int_value, right_val->u.float_value, result, line_number);
    } else if (left_val->type == CRB_FLOAT_VALUE && right_val->type == CRB_INT_VALUE) {
        eval_binary_double(inter, env, operator, left_val->u.float_value, (double)right_val->u.int_value, result, line_number);
    }
}

/*
 * 链接字符串(左操作数已经是字符串)
 */
static void chain_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *left, CRB_Value *right, CRB_Value *result) {
    CRB_Char* right_str = CRB_value_to_string(inter, env, line_number, right, NULL);
    CRB_Object* right_obj = crb_create_crowbar_string_i(inter, right_str);

    result->type = CRB_STRING_VALUE;
    int len = CRB_wcslen(left->u.object->u.string.string) + CRB_wcslen(right_obj->u.string.string);
    CRB_Char* str = MEM_malloc(sizeof(CRB_Char) * (len + 1));
    CRB_wcscpy(str, left->u.object->u.string.string);
    CRB_wcscat(str, right_obj->u.string.string);
    result->u.object = crb_create_crowbar_string_i(inter, str);
}

/*
 * 连接字符串
 */

static void eval_binary_concat_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *left, Expression *right) {
    eval_expression(inter, env, left);
    eval_expression(inter, env, right);
    CRB_Value* left_val = peek_stack(inter, 1);
    CRB_Value* right_val = peek_stack(inter, 0);

    CRB_Value result;
    CRB_Char* left_str = CRB_value_to_string(inter, env, left->line_number, left_val, NULL);
    result.u.object = crb_create_crowbar_string_i(inter, left_str);
    chain_string(inter, env, left->line_number, &result, right_val, &result);

    shrink_stack(inter, 2);
    push_value(inter, &result);
}

/*
 * 二元布尔表达式求值
 */
static CRB_Boolean eval_binary_boolean(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, CRB_Boolean left, CRB_Boolean right, int line_number) {
    CRB_Boolean result;
    if (operator == EQ_EXPRESSION) {
        result = left == right;
    } else if (operator == NE_EXPRESSION) {
        result = left != right;
    } else {
        const char* op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, line_number, NOT_BOOLEAN_OPERATOR_ERR, CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str, CRB_MESSAGE_ARGUMENT_END);
        result = CRB_FALSE;
    }
    return result;
}

/*
 * 逻辑与或表达式求值
 */
static void eval_logical_and_or_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right) {
    CRB_Value result;
    result.type = CRB_BOOLEAN_VALUE;
    eval_expression(inter, env, left);
    CRB_Value left_val;
    pop_value(inter, &left_val);
    if (left_val.type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, left->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    if (operator == LOGICAL_AND_EXPRESSION) {
        if (!left_val.u.boolean_value) {
            result.u.boolean_value = CRB_FALSE;
            goto FUNC_END;
        }
    } else if (operator == LOGICAL_OR_EXPRESSION) {
        if (left_val.u.boolean_value) {
            result.u.boolean_value = CRB_TRUE;
            goto FUNC_END;
        }
    } else {
        DBG_panic(("bad operator..%d\n", operator));
    }

    eval_expression(inter, env, right);
    CRB_Value right_val;
    pop_value(inter, &right_val);
    result.u.boolean_value = right_val.u.boolean_value;

    FUNC_END:
    push_value(inter, &result);
}

/*
 * 负数表达式求值
 */
static void eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *operand) {
    eval_expression(inter, env, operand);
    CRB_Value* operand_val = peek_stack(inter, 0);
    if (operand_val->type == CRB_INT_VALUE) {
        operand_val->u.int_value = -operand_val->u.int_value;
    } else if (operand_val->type == CRB_FLOAT_VALUE) {
        operand_val->u.float_value = -operand_val->u.float_value;
    } else {
        crb_runtime_error(inter, env, operand->line_number, MINUS_OPERAND_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

void crb_eval_minus_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *operand, CRB_Value* result) {
    eval_minus_expression(inter, env, operand);
    pop_value(inter, result);
}

/*
 * 逻辑非表达式求值
 */
static void eval_logical_not_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *operand) {
    eval_expression(inter, env, operand);
    CRB_Value* operand_val = peek_stack(inter, 0);
    if (operand_val->type != CRB_BOOLEAN_VALUE) {
        crb_runtime_error(inter, env, operand->line_number, NOT_BOOLEAN_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    operand_val->u.boolean_value = !operand_val->u.boolean_value;
}

static void dispose_ref_in_native_method(CRB_LocalEnvironment *env) {
    while (env->ref_in_native_method) {
        RefInNativeFunc* ref = env->ref_in_native_method;
        env->ref_in_native_method = ref->next;
        MEM_free(ref);
    }
}

/*
 * 释放局部作用域
 */
static void dispose_local_environment(CRB_Interpreter *inter) {
    CRB_LocalEnvironment *temp = inter->top_environment;
    inter->top_environment = inter->top_environment->next;

    if (temp->global_var_refs != NULL) {
        close_rbtree(temp->global_var_refs);
    }
    dispose_ref_in_native_method(temp);

    MEM_free(temp);
}

/*
 * 创建匿名函数值
 */
inline void CRB_create_closure(CRB_LocalEnvironment *env, CRB_FunctionDefinition *fd, CRB_Value* closure) {
    closure->type = CRB_CLOSURE_VALUE;
    closure->u.closure.function_definition = fd;
    closure->u.closure.scope_chain = env ? env->scope_chain : NULL;
}

static void call_crowbar_function_from_native(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_LocalEnvironment *caller_env, CRB_Value *func, int arg_count, CRB_Value *args, CRB_Value *result) {
    CRB_ParameterList* param_list = func->u.closure.function_definition->u.crowbar_f.parameter;
    struct lnode* param_node = param_list ? llist_front_node(param_list) : NULL;
    for (int arg_idx=0; arg_idx<arg_count; ++arg_idx, param_node=param_node->next) {
        if (param_node == NULL) {
            crb_runtime_error(inter, caller_env, line_number, ARGUMENT_TOO_MANY_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_add_local_variable(inter, env, (const char*)param_node->value.ptr_value, &args[arg_idx], CRB_FALSE);
    }
    if (param_node != NULL) {
        crb_runtime_error(inter, caller_env, line_number, ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    StatementResult stmt_result;
    crb_execute_statement_list(inter, env, func->u.closure.function_definition->u.crowbar_f.block->statement_list, &stmt_result);
    if (stmt_result.type == RETURN_STATEMENT_RESULT) {
        *result = stmt_result.u.return_value;
    } else {
        result->type = CRB_NULL_VALUE;
    }
}

static void call_native_function_from_native(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_LocalEnvironment *caller_env, CRB_Value *func, int arg_count, CRB_Value *args, CRB_Value *result) {
//    for (int i=0; i<arg_count; ++i) {
//        push_value(inter, &args[i]);
//    }
//    CRB_Value* arg_p = &inter->stack.stack[inter->stack.stack_pointer-arg_count];
    CRB_check_argument_count(inter, env, line_number, arg_count, func->u.closure.function_definition->u.native_f.param_count);
    func->u.closure.function_definition->u.native_f.func(inter, env, arg_count, args, result);
//    shrink_stack(inter, arg_count);
}

static inline void check_method_argument_count(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, int arg_count, int param_count) {
    if (param_count < 0) {
        return;
    }
    if (arg_count < param_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    } else if (arg_count > param_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_MANY_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

static inline void call_fake_method_from_native(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_LocalEnvironment *caller_env, CRB_Value *func, int arg_count, CRB_Value *args, CRB_Value *result) {
    FakeMethodDefinition* fmt = crb_search_fake_method(inter, env, line_number, &func->u.fake_method);
    check_method_argument_count(inter, env, line_number, arg_count, fmt->param_count);
//    for (int i=0; i<arg_count; ++i) {
//        push_value(inter, &args[i]);
//    }
    fmt->func(inter, env, func->u.fake_method.object, arg_count, args, result);
//    shrink_stack(inter, arg_count);
}

/*
 * 调用函数
 */
void CRB_call_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *func, int arg_count, CRB_Value *args, CRB_Value *result) {
    const char* func_name;
    CRB_Object* closure_scope_chain;
    CRB_Module* module;
    if (func->type == CRB_CLOSURE_VALUE) {
        func_name = func->u.closure.function_definition->name;
        closure_scope_chain = func->u.closure.scope_chain;
        module = func->u.closure.function_definition->module;
    } else if (func->type == CRB_FAKE_METHOD_VALUE) {
        func_name = func->u.fake_method.method_name;
        closure_scope_chain = NULL;
        module = NULL;
    } else {
        DBG_panic(("func->type..%d\n", func->type));
        func_name = NULL;
        closure_scope_chain = NULL;
        module = NULL;
    }
    CRB_LocalEnvironment* local_env = alloc_local_environment(inter, module, func_name, line_number, closure_scope_chain);
    if (func->type == CRB_CLOSURE_VALUE && func->u.closure.function_definition->is_closure && func->u.closure.function_definition->name) {
        CRB_add_assoc_member(inter, local_env->scope_chain->u.scope_chain.frame, func->u.closure.function_definition->name, func, CRB_TRUE);
    }

    int stack_pointer_backup = crb_get_stack_pointer(inter);
    RecoveryEnvironment env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        if (func->type == CRB_CLOSURE_VALUE) {
            switch (func->u.closure.function_definition->type) {
                case CRB_CROWBAR_FUNCTION_DEFINE:
                    call_crowbar_function_from_native(inter, local_env, line_number, env, func, arg_count, args, result);
                    break;
                case CRB_NATIVE_FUNCTION_DEFINE:
                    call_native_function_from_native(inter, local_env, line_number, env, func, arg_count, args, result);
                    break;
                case CRB_FUNCTION_DEFINE_TYPE_COUNT_PLUS_1:
                default:
                    DBG_assert(0, ("bad case..%d\n", func->u.closure.function_definition->type));
            }
        } else if (func->type == CRB_FAKE_METHOD_VALUE) {
            call_fake_method_from_native(inter, local_env, line_number, env, func, arg_count, args, result);
        } else {
            DBG_panic(("func->type..%d\n", func->type));
        }
    } else {
        dispose_local_environment(inter);
        crb_set_stack_pointer(inter, stack_pointer_backup);
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
    inter->current_recovery_environment = env_backup;
    dispose_local_environment(inter);
}

/*
 * 根据函数名调用函数
 */
void CRB_call_function_by_name(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *func_name, int arg_count, CRB_Value *args, CRB_Value *result) {
    CRB_FunctionDefinition* fd = CRB_search_function(inter, CRB_env_module(inter, env), func_name);
    if (fd == NULL) {
        fd = CRB_search_function(inter, NULL, func_name);
    }
    if (fd == NULL) {
        crb_runtime_error(inter, env, line_number, FUNCTION_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", func_name, CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Value func;
    CRB_create_closure(env, fd, &func);
    CRB_call_function(inter, env, line_number, &func, arg_count, args, result);
}

/*
 * 调用成员函数
 */
void CRB_call_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Object *obj, const char *method_name, int arg_count, CRB_Value *args, CRB_Value *result) {
    CRB_Value func;
    if (obj->type == STRING_OBJECT || obj->type == ARRAY_OBJECT) {
        func.type = CRB_FAKE_METHOD_VALUE;
        func.u.fake_method.method_name = method_name;
        func.u.fake_method.object = obj;
    } else if (obj->type == ASSOC_OBJECT) {
        CRB_Value* func_p = CRB_search_assoc_member(obj, method_name, NULL);
        if (func_p->type != CRB_CLOSURE_VALUE) {
            crb_runtime_error(inter, env, line_number, NOT_FUNCTION_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        func = *func_p;
    }

    push_value(inter, &func);
    CRB_call_function(inter, env, line_number, &func, arg_count, args, result);
    shrink_stack(inter, 1);
}

/*
 * 调用内置成员函数
 */
static void call_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_LocalEnvironment *caller_env, Expression *expr, CRB_FakeMethod *fm) {
    ArgumentList* arg_list = expr->u.function_call_expression.argument;
    struct lnode* node;
    int arg_count;
    if (arg_list == NULL) {
        arg_count = 0;
        node = NULL;
    } else {
        arg_count = llist_len(arg_list);
        node = llist_front_node(arg_list);
    }

    FakeMethodDefinition* fmt = crb_search_fake_method(inter, env, expr->line_number, fm);
    check_method_argument_count(inter, env, expr->line_number, arg_count, fmt->param_count);
    for (; node!=NULL; node=node->next) {
        eval_expression(inter, caller_env, (Expression*)node->value.ptr_value);
    }
    CRB_Value* args = &inter->stack.stack[inter->stack.stack_pointer-arg_count];
    CRB_Value result;
    fmt->func(inter, env, fm->object, arg_count, args, &result);
    shrink_stack(inter, arg_count);
    push_value(inter, &result);
}

/*
 * 调用自定义函数
 */
static void call_crowbar_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_LocalEnvironment *caller_env, Expression *expr, CRB_Value *func) {
    ArgumentList* arg_p = expr->u.function_call_expression.argument;  // 实参
    CRB_ParameterList* param_p = func->u.closure.function_definition->u.crowbar_f.parameter;  // 形参
    struct lnode* arg_node = arg_p ? llist_front_node(arg_p) : NULL;
    struct lnode* param_node = param_p ? llist_front_node(param_p) : NULL;
    while (arg_node != NULL) {
        if (param_node == NULL) {
            crb_runtime_error(inter, caller_env, expr->line_number, ARGUMENT_TOO_MANY_ERR, CRB_MESSAGE_ARGUMENT_END);
            return;
        }
        Expression* arg = (Expression*)arg_node->value.ptr_value;
        eval_expression(inter, caller_env, arg);
        CRB_Value* arg_val = peek_stack(inter, 0);

        const char* param_name = (const char*)param_node->value.ptr_value;
        CRB_add_local_variable(inter, env, param_name, arg_val, CRB_FALSE);
        shrink_stack(inter, 1);

        arg_node = arg_node->next;
        param_node = param_node->next;
    }
    if (param_node != NULL) {
        crb_runtime_error(inter, caller_env, expr->line_number, ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
        return;
    }

    StatementResult result;
    crb_execute_statement_list(inter, env, func->u.closure.function_definition->u.crowbar_f.block->statement_list, &result);
    if (result.type == RETURN_STATEMENT_RESULT) {
        push_value(inter, &result.u.return_value);
    } else {
        push_value(inter, &CRB_Null_Value);
    }
}

/*
 * 调用原生函数
 */
static void call_native_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_LocalEnvironment *caller_env, Expression *expr, CRB_Value *func) {
    ArgumentList* arg_list = expr->u.function_call_expression.argument;
    int arg_count;
    if (arg_list != NULL) {
        arg_count = (int)llist_len(arg_list);
        for (struct lnode* arg_node=llist_front_node(arg_list); arg_node!=NULL; arg_node=arg_node->next) {
            Expression* arg = (Expression*)arg_node->value.ptr_value;
            eval_expression(inter, caller_env, arg);
        }
    } else {
        arg_count = 0;
    }

    CRB_Value* args = &inter->stack.stack[inter->stack.stack_pointer-arg_count];
    CRB_Value result;
    CRB_check_argument_count(inter, env, expr->line_number, arg_count, func->u.closure.function_definition->u.native_f.param_count);
    func->u.closure.function_definition->u.native_f.func(inter, env, arg_count, args, &result);
    shrink_stack(inter, arg_count);
    push_value(inter, &result);
}

/*
 * 调用函数
 */
static void do_function_call(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_LocalEnvironment *caller_env, Expression *expr, CRB_Value *func) {
    if (func->type == CRB_FAKE_METHOD_VALUE) {
        call_fake_method(inter, env, caller_env, expr, &func->u.fake_method);
        return;
    }

    DBG_assert(func->type == CRB_CLOSURE_VALUE, ("func->type..%d\n", func->type));
    switch (func->u.closure.function_definition->type) {
        case CRB_CROWBAR_FUNCTION_DEFINE:
            call_crowbar_function(inter, env, caller_env, expr, func);
            return;
        case CRB_NATIVE_FUNCTION_DEFINE:
            call_native_function(inter, env, caller_env, expr, func);
            return;
        case CRB_FUNCTION_DEFINE_TYPE_COUNT_PLUS_1:
        default:
            DBG_assert(0, ("bad case..%d\n", func->u.closure.function_definition->type));
    }
}

/*
 * 函数调用表达式求值
 */
static void eval_function_call_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    const char* func_name;
    CRB_Object* closure_scope_chain;
    CRB_Module* module;
    eval_expression(inter, env, expr->u.function_call_expression.function);
    CRB_Value* func = peek_stack(inter, 0);
    if (func->type == CRB_CLOSURE_VALUE) {
        func_name = func->u.closure.function_definition->name;
        closure_scope_chain = func->u.closure.scope_chain;
        module = func->u.closure.function_definition->module;
    } else if (func->type == CRB_FAKE_METHOD_VALUE) {
        func_name = func->u.fake_method.method_name;
        closure_scope_chain = NULL;
        module = NULL;
    } else {
        crb_runtime_error(inter, env, expr->line_number, NOT_FUNCTION_ERR, CRB_MESSAGE_ARGUMENT_END);
        func_name = NULL;
        closure_scope_chain = NULL;
        module = NULL;
    }

    CRB_LocalEnvironment* local_env = alloc_local_environment(inter, module, func_name, expr->line_number, closure_scope_chain);
    if (func->type == CRB_CLOSURE_VALUE && func->u.closure.function_definition->is_closure && func->u.closure.function_definition->name) {
        CRB_add_assoc_member(inter, local_env->scope_chain->u.scope_chain.frame, func->u.closure.function_definition->name, func, CRB_TRUE);
    }

    int stack_pointer_backup = crb_get_stack_pointer(inter);
    RecoveryEnvironment env_backup = inter->current_recovery_environment;
    if (setjmp(inter->current_recovery_environment.environment) == 0) {
        do_function_call(inter, local_env, env, expr, func);
    } else {
        dispose_local_environment(inter);
        crb_set_stack_pointer(inter, stack_pointer_backup);
        longjmp(env_backup.environment, LONGJMP_ARG);
    }
    inter->current_recovery_environment = env_backup;
    dispose_local_environment(inter);

    CRB_Value return_value;
    pop_value(inter, &return_value);
    shrink_stack(inter, 1); /* func */
    push_value(inter, &return_value);
}

/*
 * 成员表达式求值
 */
static void eval_member_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.member_expression.expression);
    CRB_Value left;
    pop_value(inter, &left);
    switch (left.type) {
        case CRB_ASSOC_VALUE:
            // 关联数组成员
        {
            CRB_Value *value_p = CRB_search_assoc_member(left.u.object, expr->u.member_expression.member_name, NULL);
            if (value_p != NULL) {
                push_value(inter, value_p);
                return;
            }
            crb_runtime_error(inter, env, expr->line_number, NO_SUCH_MEMBER_ERR, CRB_STRING_MESSAGE_ARGUMENT, "member_name", expr->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
            return;
        }
        case CRB_STRING_VALUE:  /* FALLTHRU */
        case CRB_ARRAY_VALUE:
            // 字符串或数组的内置成员方法
        {
            CRB_Value value;
            value.type = CRB_FAKE_METHOD_VALUE;
            value.u.fake_method.method_name = expr->u.member_expression.member_name;
            value.u.fake_method.object = left.u.object;
            push_value(inter, &value);
            return;
        }
        case CRB_MODULE_VALUE:
            // 模块内全局变量或全局函数
        {
            CRB_FunctionDefinition* fd = CRB_search_function(inter, left.u.module, expr->u.member_expression.member_name);
            if (fd != NULL) {
                CRB_Value func;
                CRB_create_closure(NULL, fd, &func);
                push_value(inter, &func);
                return;
            }
            CRB_Value* value_p = CRB_search_global_variable(inter, left.u.module, expr->u.member_expression.member_name, NULL);
            if (value_p != NULL) {
                push_value(inter, value_p);
                return;
            }

            crb_runtime_error(inter, env, expr->line_number, IDENTIFIER_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);

        }
        default:
            crb_runtime_error(inter, env, expr->line_number, NO_MEMBER_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

/*
 * 比较字符串布尔表达求值
 */
static CRB_Boolean eval_compare_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number) {
    CRB_Boolean result;
    int cmp = CRB_wcscmp(left->u.object->u.string.string, right->u.object->u.string.string);
    if (operator == EQ_EXPRESSION) {
        return cmp == 0;
    } else if (operator == NE_EXPRESSION) {
        return cmp != 0;
    } else if (operator == GT_EXPRESSION) {
        return cmp > 0;
    } else if (operator == GE_EXPRESSION) {
        return cmp >= 0;
    } else if (operator == LT_EXPRESSION) {
        return cmp < 0;
    } else if (operator == LE_EXPRESSION) {
        return cmp <= 0;
    } else {
        const char* op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, line_number, BAD_OPERATOR_FOR_STRING_ERR, CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str, CRB_MESSAGE_ARGUMENT_END);
        result = CRB_FALSE;
    }
    return result;
}

/*
 * NULL比较布尔表达式求值
 */
static CRB_Boolean eval_binary_null(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, CRB_Value *left, CRB_Value *right, int line_number) {
    CRB_Boolean result;
    if (operator == EQ_EXPRESSION) {
        result = left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE;
    } else if (operator == NE_EXPRESSION) {
        result = !(left->type == CRB_NULL_VALUE && right->type == CRB_NULL_VALUE);
    } else {
        const char* op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, line_number, NOT_NULL_OPERATOR_ERR, CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str, CRB_MESSAGE_ARGUMENT_END);
        result = CRB_FALSE;
    }
    return result;
}

/*
 * 二元表达式求值
 */
static void eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right) {
    eval_expression(inter, env, left);
    eval_expression(inter, env, right);
    CRB_Value* left_val = peek_stack(inter, 1);
    CRB_Value* right_val = peek_stack(inter, 0);

    CRB_Value result;
    if (crb_is_numeric_type(left_val->type) && crb_is_numeric_type(right_val->type)) {
        eval_binary_numeric(inter, env, operator, left_val, right_val, &result, left->line_number);

    } else if (left_val->type == CRB_BOOLEAN_VALUE && right_val->type == CRB_BOOLEAN_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_boolean(inter, env, operator, left_val->u.boolean_value, right_val->u.boolean_value, left->line_number);

    } else if (left_val->type == CRB_STRING_VALUE && operator == ADD_EXPRESSION) {
        chain_string(inter, env, right->line_number, left_val, right_val, &result);

    } else if (left_val->type == CRB_STRING_VALUE && right_val->type == CRB_STRING_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_compare_string(inter, env, operator, left_val, right_val, left->line_number);

    } else if (left_val->type == CRB_NULL_VALUE || right_val->type == CRB_NULL_VALUE) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = eval_binary_null(inter, env, operator, left_val, right_val, left->line_number);

    } else if (crb_is_object_value(left_val->type) && crb_is_object_value(right_val->type)) {
        result.type = CRB_BOOLEAN_VALUE;
        result.u.boolean_value = (left_val->u.object == right_val->u.object);

    } else {
        const char* op_str = crb_get_operator_string(operator);
        crb_runtime_error(inter, env, left->line_number, BAD_OPERAND_TYPE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "operator", op_str, CRB_MESSAGE_ARGUMENT_END);
    }
    shrink_stack(inter, 2);
    push_value(inter, &result);
}

void crb_eval_binary_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, ExpressionType operator, Expression *left, Expression *right, CRB_Value *result) {
    eval_binary_expression(inter, env, operator, left, right);
    pop_value(inter, result);
}

/*
 * 赋值
 */
static void do_assign(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Value *src, CRB_Value *dest, AssignmentOperator operator, int line_number) {
    ExpressionType expr_type;
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
                expr_type = EXPRESSION_TYPE_COUNT_PLUS_1;
        }
        CRB_Value result;
        if (dest->type == CRB_STRING_VALUE && expr_type == ADD_EXPRESSION) {
            chain_string(inter, env, line_number, dest, src, &result);
        } else {
            eval_binary_numeric(inter, env, expr_type, dest, src, &result, line_number);
        }
        *dest = result;
    }
}

/*
 * 关联数组赋值
 * expr: left = rvalue
 *       expression.member_name = rvalue
 * src: rvalue
 */
static void assign_to_member(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr, CRB_Value *src) {
    Expression *left = expr->u.assign_expression.left;
    eval_expression(inter, env, left->u.member_expression.expression);
    CRB_Value* assoc = peek_stack(inter, 0);
    if (assoc->type == CRB_ASSOC_VALUE) {
        CRB_Boolean is_final;
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
    } else if (assoc->type == CRB_MODULE_VALUE) {
        CRB_Boolean is_final;
        CRB_Value* dest = CRB_search_global_variable(inter, assoc->u.module, left->u.member_expression.member_name, &is_final);
        if (dest == NULL) {
            if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
                crb_runtime_error(inter, env, expr->line_number, IDENTIFIER_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
            }
            CRB_FunctionDefinition* fd = CRB_search_function(inter, assoc->u.module, left->u.member_expression.member_name);
            if (fd != NULL) {
                crb_runtime_error(inter, env, expr->line_number, FUNCTION_EXISTS_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
            }
            CRB_add_global_variable(inter, assoc->u.module, left->u.member_expression.member_name, src, expr->u.assign_expression.is_final);
        } else {
            if (is_final) {
                crb_runtime_error(inter, env, expr->line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
            }
            do_assign(inter, env, src, dest, expr->u.assign_expression.operator, expr->line_number);
        }
    } else {
        crb_runtime_error(inter, env, expr->line_number, NOT_OBJECT_MEMBER_ASSIGN_ERR, CRB_MESSAGE_ARGUMENT_END);
    }


    shrink_stack(inter, 1);
}

/*
 * 获取标识符左值
 */
CRB_Value* crb_get_identifier_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *identifier) {
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
static inline CRB_Value* get_array_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr, CRB_Value* array) {
    eval_expression(inter, env, expr->u.index_expression.index);
    CRB_Value index;
    pop_value(inter, &index);
    if (index.type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Value_SLICE* arr = array->u.object->u.array.array;
    long len = CRB_Value_slice_len(arr);
    long idx = index.u.int_value;
    if (idx < 0 || idx >= len) {
        crb_runtime_error(inter, env, expr->line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", len, CRB_INT_MESSAGE_ARGUMENT, "index", idx, CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Value* data = CRB_Value_slice_data(arr);
    return data + idx;
}

/*
 * 获取角标元素左值地址
 */
static inline CRB_Value* get_element_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.index_expression.array);
    CRB_Value array;
    pop_value(inter, &array);
    if (array.type != CRB_ARRAY_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_ARRAY_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    return get_array_element_lvalue(inter, env, expr, &array);
}

/*
 * 获取关联数组成员左值地址
 */
static CRB_Value* get_member_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.member_expression.expression);
    CRB_Value assoc;
    pop_value(inter, &assoc);
    if (assoc.type == CRB_ASSOC_VALUE) {
        CRB_Boolean is_final = CRB_FALSE;
        CRB_Value* dest = CRB_search_assoc_member(assoc.u.object, expr->u.member_expression.member_name, &is_final);
        if (is_final) {
            crb_runtime_error(inter, env, expr->line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
        }
        return dest;
    } else if (assoc.type == CRB_MODULE_VALUE) {
        CRB_Boolean is_final = CRB_FALSE;
        CRB_Value* dest = CRB_search_global_variable(inter, assoc.u.module, expr->u.member_expression.member_name, &is_final);
        if (is_final) {
            crb_runtime_error(inter, env, expr->line_number, ASSIGN_TO_FINAL_VARIABLE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", expr->u.member_expression.member_name, CRB_MESSAGE_ARGUMENT_END);
        }
        return dest;
    } else {
        crb_runtime_error(inter, env, expr->line_number, NOT_OBJECT_MEMBER_UPDATE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    return NULL;
}

/*
 * 获取左值地址
 */
static CRB_Value* get_lvalue(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    CRB_Value* dest;
    switch (expr->type) {
        case IDENTIFIER_EXPRESSION:
            dest = crb_get_identifier_lvalue(inter, env, expr->line_number, expr->u.identifier);
            return dest;
        case INDEX_EXPRESSION:
            dest = get_element_lvalue(inter, env, expr);
            return dest;
        case MEMBER_EXPRESSION:
            dest = get_member_lvalue(inter, env, expr);
            return dest;
        default:
            crb_runtime_error(inter, env, expr->line_number, NOT_LVALUE_ERR, CRB_MESSAGE_ARGUMENT_END);
            dest = NULL;
    }
    return dest;
}

/*
 * 截取字符串求值
 */
static inline void eval_sub_string_value(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr, CRB_Value *string, CRB_Boolean just_index, Expression *start_expr, Expression *end_expr) {
    long start_value;
    if (start_expr == NULL) {
        start_value = 0;
    } else {
        eval_expression(inter, env, start_expr);
        CRB_Value start;
        pop_value(inter, &start);
        if (start.type != CRB_INT_VALUE) {
            crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        start_value = start.u.int_value;
    }

    CRB_Char* str = string->u.object->u.string.string;
    long len = CRB_wcslen(str);

    long end_value;
    if (just_index) {
        end_value = start_value + 1;
    } else {
        if (end_expr == NULL) {
            end_value = len;
        } else {
            eval_expression(inter, env, end_expr);
            CRB_Value end;
            pop_value(inter, &end);
            if (end.type != CRB_INT_VALUE) {
                crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
            }
            end_value = end.u.int_value;
        }
    }

    CRB_Value value;
    value.type = CRB_STRING_VALUE;
    value.u.object = crb_string_substr_i(inter, env, string->u.object, start_value, end_value - start_value, expr->line_number);
    push_value(inter, &value);
}

/*
 * 索引数组表达式求值，作为直接左值的时候不会被调用，取而代之将调用get_element_lvalue
 */
static void eval_index_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.index_expression.array);
    CRB_Value array;
    pop_value(inter, &array);

    CRB_Value* value;
    switch (array.type) {
        case CRB_ARRAY_VALUE:
            value = get_array_element_lvalue(inter, env, expr, &array);
            push_value(inter, value);
            return;
        case CRB_STRING_VALUE:
            eval_sub_string_value(inter, env, expr, &array, CRB_TRUE, expr->u.index_expression.index, NULL);
            return;
        default:
            crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_SUPPORT_INDEXING_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

/*
 * 数组切片表达式求值
 */
static void eval_array_slice_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression* expr, CRB_Value* array, Expression *start_expr, Expression *end_expr) {
    long start_value;
    if (start_expr == NULL) {
        start_value = 0;
    } else {
        eval_expression(inter, env, start_expr);
        CRB_Value start;
        pop_value(inter, &start);
        if (start.type != CRB_INT_VALUE) {
            crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        start_value = start.u.int_value;
    }

    CRB_Value_SLICE* arr = array->u.object->u.array.array;

    long end_value;
    if (end_expr == NULL) {
        end_value = CRB_Value_slice_len(arr);
    } else {
        eval_expression(inter, env, end_expr);
        CRB_Value end;
        pop_value(inter, &end);
        if (end.type != CRB_INT_VALUE) {
            crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_INT_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        end_value = end.u.int_value;
    }

    long cap = CRB_Value_slice_cap(arr);
    long pos = CRB_Value_slice_pos(arr);
    if (start_value < 0 || start_value > end_value || pos + end_value > cap) {
        crb_runtime_error(inter, env, expr->line_number, ARRAY_SLICE_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", cap, CRB_INT_MESSAGE_ARGUMENT, "begin", start_value, CRB_INT_MESSAGE_ARGUMENT, "end", end_value, CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Value v;
    v.type = CRB_ARRAY_VALUE;
    v.u.object = crb_create_array_slice_i(inter, arr, start_value, end_value);
    push_value(inter, &v);
}

/*
 * 切片表达式求值
 */
static void eval_slice_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.slice_expression.array);
    CRB_Value array;
    pop_value(inter, &array);
    switch (array.type) {
        case CRB_ARRAY_VALUE:
            eval_array_slice_expression(inter, env, expr, &array, expr->u.slice_expression.start, expr->u.slice_expression.end);
            return;
        case CRB_STRING_VALUE:
            eval_sub_string_value(inter, env, expr, &array, CRB_FALSE, expr->u.slice_expression.start, expr->u.slice_expression.end);
            return;
        default:
            crb_runtime_error(inter, env, expr->line_number, INDEX_OPERAND_NOT_SUPPORT_INDEXING_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

/*
 * 自增自减表达式求值
 */
static void eval_inc_dec_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    CRB_Value   result;
    CRB_Value* operand = get_lvalue(inter, env, expr->u.inc_dec.operand);
    if (operand == NULL) {
        crb_runtime_error(inter, env, expr->line_number, INC_DEC_OPERAND_NOT_EXIST_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    if (operand->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, expr->line_number, INC_DEC_OPERAND_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    int old_value = operand->u.int_value;
    if (expr->type == INCREMENT_EXPRESSION) {
        operand->u.int_value++;
    } else {
        DBG_assert(expr->type == DECREMENT_EXPRESSION, ("expr->type..%d\n", expr->type));
        operand->u.int_value--;
    }

    result.type = CRB_INT_VALUE;
    result.u.int_value = old_value;
    push_value(inter, &result);
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

    // 获取左值地址
    CRB_Value* dest = get_lvalue(inter, env, left);
    if (dest != NULL) {
        do_assign(inter, env, src, dest, expr->u.assign_expression.operator, expr->line_number);
    } else {
        // 左值不存在，可能需要创建变量
        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
            crb_runtime_error(inter, env, expr->line_number, IDENTIFIER_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
        }
        if (left->type == IDENTIFIER_EXPRESSION) {
            if (env != NULL) {
                CRB_add_local_variable(inter, env, left->u.identifier, src, expr->u.assign_expression.is_final);
            } else {
                CRB_FunctionDefinition* fd = CRB_search_function(inter, CRB_env_module(inter, env), left->u.identifier);
                if (fd == NULL) {
                    fd = CRB_search_function(inter, NULL, left->u.identifier);
                }
                if (fd != NULL) {
                    crb_runtime_error(inter, env, expr->line_number, FUNCTION_EXISTS_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
                }
                CRB_add_global_variable(inter, CRB_env_module(inter, env), left->u.identifier, src, expr->u.assign_expression.is_final);
            }
        } else {
            DBG_assert(0, ("bad expression type: %d\n", left->type));
        }
    }






//    if (left->type == IDENTIFIER_EXPRESSION && dest == NULL) {
//        if (expr->u.assign_expression.operator != NORMAL_ASSIGN) {
//            crb_runtime_error(inter, env, expr->line_number, IDENTIFIER_NOT_FOUND_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
//        }
//        if (env != NULL) {
//            CRB_add_local_variable(inter, env, left->u.identifier, src, expr->u.assign_expression.is_final);
//        } else {
//            CRB_FunctionDefinition* fd = CRB_search_function(inter, CRB_env_module(inter, env), left->u.identifier);
//            if (fd == NULL) {
//                fd = CRB_search_function(inter, NULL, left->u.identifier);
//            }
//            if (fd != NULL) {
//                crb_runtime_error(inter, env, expr->line_number, FUNCTION_EXISTS_ERR, CRB_STRING_MESSAGE_ARGUMENT, "name", left->u.identifier, CRB_MESSAGE_ARGUMENT_END);
//            }
//            CRB_add_global_variable(inter, CRB_env_module(inter, env), left->u.identifier, src, expr->u.assign_expression.is_final);
//        }
//    } else {
//        DBG_assert(dest != NULL, ("dest == NULL.\n"));
//        do_assign(inter, env, src, dest, expr->u.assign_expression.operator, expr->line_number);
//    }
}

/*
 * 逗号表达式求值
 */
static void eval_comma_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr->u.comma.left);
    shrink_stack(inter, 1);
    eval_expression(inter, env, expr->u.comma.right);
}

/*
 * 字符串字面量转换为字符串变量
 */
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
            eval_double_expression(inter, expr->u.float_value);
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
        case CONCAT_STRING_EXPRESSION:
            eval_binary_concat_string(inter, env, expr->u.binary_expression.left, expr->u.binary_expression.right);
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
        case ASSOC_EXPRESSION:
            eval_assoc_expression(inter, env, expr->u.assoc_literal);
            break;
        case CLOSURE_EXPRESSION:
            eval_closure_expression(inter, env, expr);
            break;
        case INDEX_EXPRESSION:
            eval_index_expression(inter, env, expr);
            break;
        case SLICE_EXPRESSION:
            eval_slice_expression(inter, env, expr);
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

void crb_eval_expression(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr, CRB_Value *result) {
    eval_expression(inter, env, expr);
    pop_value(inter, result);
}

CRB_Value* crb_eval_expression_peek(CRB_Interpreter *inter, CRB_LocalEnvironment *env, Expression *expr) {
    eval_expression(inter, env, expr);
    return peek_stack(inter, 0);
}
