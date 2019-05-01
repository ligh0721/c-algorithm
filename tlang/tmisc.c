//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "tinterpreter.h"
#include "tmisc.h"
#include "terror.h"


/*
 * CRB_dispose_interpreter时统一释放
 */
void* crb_malloc(size_t size) {
    CRB_Interpreter* inter = crb_get_current_interpreter();
    return MEM_storage_malloc(inter->interpreter_storage, size);
}

void* crb_execute_malloc(CRB_Interpreter *inter, size_t size) {
    return MEM_storage_malloc(inter->execute_storage, size);
}

const char* crb_get_operator_string(ExpressionType type) {
    const char* str;
    switch (type) {
        case BOOLEAN_EXPRESSION:    /* FALLTHRU */
        case INT_EXPRESSION:        /* FALLTHRU */
        case DOUBLE_EXPRESSION:     /* FALLTHRU */
        case STRING_EXPRESSION:     /* FALLTHRU */
//        case REGEXP_EXPRESSION:     /* FALLTHRU */
        case IDENTIFIER_EXPRESSION:
            DBG_assert(0, ("bad expression type..%d\n", type));
            str = NULL;
            break;
        case COMMA_EXPRESSION:
            str = ",";
            break;
        case ASSIGN_EXPRESSION:
            str = "=";
            break;
        case CONCAT_STRING_EXPRESSION:
            str = "..";
            break;
        case ADD_EXPRESSION:
            str = "+";
            break;
        case SUB_EXPRESSION:
            str = "-";
            break;
        case MUL_EXPRESSION:
            str = "*";
            break;
        case DIV_EXPRESSION:
            str = "/";
            break;
        case MOD_EXPRESSION:
            str = "%";
            break;
        case EQ_EXPRESSION:
            str = "==";
            break;
        case NE_EXPRESSION:
            str = "!=";
            break;
        case GT_EXPRESSION:
            str = "<";
            break;
        case GE_EXPRESSION:
            str = "<=";
            break;
        case LT_EXPRESSION:
            str = ">";
            break;
        case LE_EXPRESSION:
            str = ">=";
            break;
        case LOGICAL_AND_EXPRESSION:
            str = "&&";
            break;
        case LOGICAL_OR_EXPRESSION:
            str = "||";
            break;
        case MINUS_EXPRESSION:
            str = "-";
            break;
        case LOGICAL_NOT_EXPRESSION:
            str = "!";
            break;
        case FUNCTION_CALL_EXPRESSION:      /* FALLTHRU */
        case MEMBER_EXPRESSION:     /* FALLTHRU */
        case NULL_EXPRESSION:       /* FALLTHRU */
        case ARRAY_EXPRESSION:      /* FALLTHRU */
        case ASSOC_EXPRESSION:      /* FALLTHRU */
        case INDEX_EXPRESSION:      /* FALLTHRU */
        case SLICE_EXPRESSION:      /* FALLTHRU */
        case INCREMENT_EXPRESSION:  /* FALLTHRU */
        case DECREMENT_EXPRESSION:  /* FALLTHRU */
        case CLOSURE_EXPRESSION:    /* FALLTHRU */
        case EXPRESSION_TYPE_COUNT_PLUS_1:  /* FALLTHRU */
        default:
            DBG_assert(0, ("bad expression type..%d\n", type));
            str = NULL;
    }
    return str;
}

const char* CRB_get_type_name(CRB_ValueType type) {
    switch (type) {
        case CRB_BOOLEAN_VALUE:
            return "boolean";
            break;
        case CRB_INT_VALUE:
            return "int";
            break;
        case CRB_FLOAT_VALUE:
            return "float";
            break;
        case CRB_STRING_VALUE:
            return "string";
            break;
        case CRB_NATIVE_POINTER_VALUE:
            return "native pointer";
            break;
        case CRB_NULL_VALUE:
            return "null";
            break;
        case CRB_ARRAY_VALUE:
            return "array";
            break;
        case CRB_ASSOC_VALUE:
            return "object";
            break;
        case CRB_CLOSURE_VALUE:
            return "closure";
            break;
        case CRB_FAKE_METHOD_VALUE:
            return "method";
            break;
        case CRB_SCOPE_CHAIN_VALUE:
            return "scope chain";
            break;
        default:
            DBG_panic(("bad type..%d\n", type));
    }
    return NULL; /* make compiler happy */
}

// vstring
static inline void vstr_grow(VString *v, long mincap) {
    long new_cap = v->cap << 1;
    if (new_cap < mincap) {
        new_cap = mincap;
    }
    DBG_assert(new_cap > 0, ("vstr out of memory.\n"));
    v->string = MEM_realloc(v->string, sizeof(CRB_Char)*new_cap);
    v->cap = new_cap;
}

void crb_vstr_init(VString *v) {
    v->string = NULL;
    v->cap = v->len = 0;
}

void crb_vstr_append_string(VString *v, const CRB_Char *str) {
    int mincap = v->len + CRB_wcslen(str) + 1;
    if (mincap > v->cap) {
        vstr_grow(v, mincap);
    }
    CRB_wcscpy(v->string+v->len, str);
    v->len = mincap - 1;
}

void crb_vstr_append_character(VString *v, CRB_Char ch) {
    int mincap = v->len + 2;
    if (mincap > v->cap) {
        vstr_grow(v, mincap);
    }
    v->string[v->len] = ch;
    v->string[v->len+1] = 0;
    v->len = mincap - 1;
}

CRB_Value* CRB_add_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    RBTREE* global_vars = CRB_global_vars(inter, module);
    Variable* new_var = crb_execute_malloc(inter, sizeof(Variable));
    new_var->is_final = is_final;
    char* name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(name, identifier);
    new_var->name = name;
    new_var->value = *value;
    rbtree_set(global_vars, ptr_value(new_var));
    return &new_var->value;
}

Variable* crb_search_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier) {
    RBTREE* global_vars = CRB_global_vars(inter, module);
    if (global_vars == NULL) {
        return NULL;
    }
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(global_vars, ptr_value(&key), NULL);
    return (Variable*)res.ptr_value;
}

CRB_Value* CRB_search_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier, CRB_Boolean *is_final) {
    RBTREE* global_vars = CRB_global_vars(inter, module);
    if (global_vars == NULL) {
        return NULL;
    }
    int ok;
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(global_vars, ptr_value(&key), &ok);
    if (ok) {
        Variable* var = (Variable*)res.ptr_value;
        if (is_final) {
            *is_final = var->is_final;
        }
        return &var->value;
    }
    return NULL;
}

CRB_Value* CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env, const char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    DBG_assert(env->scope_chain->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->scope_chain->type));
    CRB_Value* ret = CRB_add_assoc_member(inter, env->scope_chain->u.scope_chain.frame, identifier, value, is_final);
    return ret;
}

CRB_Value* CRB_search_local_variable(CRB_LocalEnvironment *env, const char *identifier, CRB_Boolean *is_final) {
    if (env == NULL) {
        return NULL;
    }
    DBG_assert(env->scope_chain->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->scope_chain->type));

    for (CRB_Object* sc=env->scope_chain; sc; sc=sc->u.scope_chain.next) {
        DBG_assert(sc->type == SCOPE_CHAIN_OBJECT, ("sc->type..%d\n", sc->type));
        CRB_Value* value = CRB_search_assoc_member(sc->u.scope_chain.frame, identifier, is_final);
        if (value) {
            return value;
        }
    }
    return NULL;
}

CRB_FunctionDefinition* CRB_search_function(CRB_Interpreter *inter, CRB_Module* module, const char *name) {
    RBTREE* global_funcs = CRB_global_funcs(inter, module);
    NamedItemEntry key = {name};
    VALUE res = rbtree_get(global_funcs, ptr_value(&key), NULL);
    return (CRB_FunctionDefinition*)res.ptr_value;
}

CRB_FunctionDefinition* crb_search_function_in_compile(const char *name) {
    CRB_Interpreter* inter = crb_get_current_interpreter();
    return CRB_search_function(inter, inter->current_module, name);
}

struct _fake_method_entry {
    ObjectType  type;
    const char  *name;
};

FakeMethodDefinition* crb_search_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_FakeMethod *fm) {
    struct _fake_method_entry key = {fm->object->type, fm->method_name};
    VALUE res = rbtree_get(inter->fake_methods, ptr_value(&key), NULL);
    FakeMethodDefinition* ret = (FakeMethodDefinition*)res.ptr_value;
    if (ret == NULL) {
        crb_runtime_error(inter, env, line_number, NO_SUCH_METHOD_ERR, CRB_STRING_MESSAGE_ARGUMENT, "method_name", fm->method_name, CRB_MESSAGE_ARGUMENT_END);
    }
    return ret;
}

CRB_Module* CRB_add_module(CRB_Interpreter* inter, const char* name) {
    CRB_Module* module = (CRB_Module*)crb_malloc(sizeof(CRB_Module));
    module->name = name;
    module->global_vars = open_rbtree(_crb_asc_order_named_item);
    module->global_funcs = open_rbtree(_crb_asc_order_named_item);
    rbtree_set(inter->modules, ptr_value(module));
    return module;
}

CRB_Module* CRB_add_module_if_not_exist(CRB_Interpreter* inter, const char* name, CRB_Boolean* exist) {
    CRB_Module* module;
    RBNODE* parent;
    NamedItemEntry key = {name};
    RBNODE** where = rbtree_fast_get(inter->modules, ptr_value(&key), &parent);
    if (rbtree_node_not_found(inter->modules, where)) {
        module = (CRB_Module*)crb_malloc(sizeof(CRB_Module));
        module->name = name;
        module->global_vars = open_rbtree(_crb_asc_order_named_item);
        module->global_funcs = open_rbtree(_crb_asc_order_named_item);
        RBNODE *node = rbtree_open_node(inter->modules, ptr_value(module), parent);
        rbtree_fast_set(inter->modules, where, node);
        if (exist != NULL) {
            *exist = CRB_FALSE;
        }
    } else {
        module = (CRB_Module*)rbtree_fast_value(inter->modules, where)->ptr_value;
        if (exist != NULL) {
            *exist = CRB_TRUE;
        }
    }
    return module;
}

CRB_Module* CRB_search_module(CRB_Interpreter* inter, const char* name) {
    NamedItemEntry key = {name};
    VALUE res = rbtree_get(inter->modules, ptr_value(&key), NULL);
    return (CRB_Module*)res.ptr_value;
}

struct _value_to_string_params {
    VString     vstr;
    char        buf[LINE_BUF_SIZE];
    CRB_Char    wc_buf[LINE_BUF_SIZE];
    RBTREE*     record;
    long        deep;
};

static inline int check_record(const CRB_Value *value, struct _value_to_string_params* params) {
    if (value->type == CRB_ARRAY_VALUE || value->type == CRB_ASSOC_VALUE) {
        VALUE v = ptr_value((void*)value->u.object);
        RBNODE* parent;
        RBNODE** where = rbtree_fast_get(params->record, v, &parent);
        if (rbtree_node_not_found(params->record, where)) {
            RBNODE* node = rbtree_open_node(params->record, v, parent);
            rbtree_fast_set(params->record, where, node);
            return 0;
        } else {
            if (value->type == CRB_ARRAY_VALUE) {
                CRB_mbstowcs("[...]", params->wc_buf);
            } else if (value->type == CRB_ASSOC_VALUE) {
                CRB_mbstowcs("{...}", params->wc_buf);
            }
            return 1;
        }
    }
    return 0;
}

struct _assoc_every_member_to_string_params {
    CRB_Boolean need_comma;
    CRB_Interpreter* inter;
    CRB_LocalEnvironment *env;
    int line_number;
    struct _value_to_string_params* params;
};

static int _assoc_every_member_to_string(const AssocMember* value, void* param) {
    struct _assoc_every_member_to_string_params* params = (struct _assoc_every_member_to_string_params*)param;
    if (params->need_comma == CRB_TRUE) {
        crb_vstr_append_string(&params->params->vstr, L", ");
    } else {
        params->need_comma = CRB_TRUE;
    }

    CRB_Char* new_str = CRB_mbstowcs_alloc(params->inter, params->env, params->line_number, value->name);
    DBG_assert(new_str != NULL, ("new_str is null.\n"));
    if (value->is_final) {
        crb_vstr_append_string(&params->params->vstr, L"final ");
    }
    crb_vstr_append_string(&params->params->vstr, new_str);
    MEM_free(new_str);

    crb_vstr_append_string(&params->params->vstr, L": ");
    if (check_record(&value->value, params->params)) {
        crb_vstr_append_string(&params->params->vstr, params->params->wc_buf);
    } else {
        CRB_value_to_string(params->inter, params->env, params->line_number, &value->value, params->params);
    }
    return 0;
}

CRB_Char* CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const CRB_Value *value, void* param) {
    struct _value_to_string_params* params = (struct _value_to_string_params*)param;
    if (params == NULL) {
        params = (struct _value_to_string_params*)MEM_malloc(sizeof(struct _value_to_string_params));
        crb_vstr_init(&params->vstr);
        params->record = open_rbtree(asc_order_int);
        params->deep = 0;
    }
    params->deep++;
    check_record(value, params);

    switch (value->type) {
        case CRB_BOOLEAN_VALUE:
            if (value->u.boolean_value) {
                crb_vstr_append_string(&params->vstr, L"true");
            } else {
                crb_vstr_append_string(&params->vstr, L"false");
            }
            break;
        case CRB_INT_VALUE:
            sprintf(params->buf, "%ld", value->u.int_value);
            CRB_mbstowcs(params->buf, params->wc_buf);
            crb_vstr_append_string(&params->vstr, params->wc_buf);
            break;
        case CRB_FLOAT_VALUE:
            sprintf(params->buf, "%lf", value->u.float_value);
            CRB_mbstowcs(params->buf, params->wc_buf);
            crb_vstr_append_string(&params->vstr, params->wc_buf);
            break;
        case CRB_STRING_VALUE:
            crb_vstr_append_string(&params->vstr, value->u.object->u.string.string);
            break;
        case CRB_NATIVE_POINTER_VALUE:
            sprintf(params->buf, "%s(%p)", value->u.object->u.native_pointer.info->name, value->u.object->u.native_pointer.pointer);
            CRB_mbstowcs(params->buf, params->wc_buf);
            crb_vstr_append_string(&params->vstr, params->wc_buf);
            break;
        case CRB_NULL_VALUE:
            CRB_mbstowcs("null", params->wc_buf);
            crb_vstr_append_string(&params->vstr, params->wc_buf);
            break;
        case CRB_ARRAY_VALUE:
            crb_vstr_append_character(&params->vstr, L'[');
            CRB_Value_SLICE* arr = value->u.object->u.array.array;
            long len = CRB_Value_slice_len(arr);
            CRB_Value* data = CRB_Value_slice_data(arr);
            for (long i = 0; i<len; ++i) {
                if (i > 0) {
                    crb_vstr_append_string(&params->vstr, L", ");
                }
                if (check_record(data+i, params)) {
                    crb_vstr_append_string(&params->vstr, params->wc_buf);
                } else {
                    CRB_value_to_string(inter, env, line_number, data+i, params);
                }
            }
            crb_vstr_append_character(&params->vstr, L']');
            break;
        case CRB_ASSOC_VALUE:
            crb_vstr_append_character(&params->vstr, L'{');
            AssocMember_RBTREE* tr = value->u.object->u.assoc.members;
            struct _assoc_every_member_to_string_params _params = {CRB_FALSE, inter, env, line_number, params};
            AssocMember_rbtree_ldr(tr, _assoc_every_member_to_string, &_params);
            crb_vstr_append_character(&params->vstr, L'}');
            break;
        case CRB_CLOSURE_VALUE:
            crb_vstr_append_string(&params->vstr, L"closure(");
            if (value->u.closure.function_definition->name == NULL) {
                crb_vstr_append_string(&params->vstr, L"null");
            } else {
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.closure.function_definition->name);
                DBG_assert(new_str != NULL, ("new_str is null.\n"));
                crb_vstr_append_string(&params->vstr, new_str);
                MEM_free(new_str);
            }
            crb_vstr_append_character(&params->vstr, L')');
            break;
        case CRB_FAKE_METHOD_VALUE:
            crb_vstr_append_string(&params->vstr, L"fake_method(");
            {
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.fake_method.method_name);
                DBG_assert(new_str != NULL, ("new_str is null.\n"));
                crb_vstr_append_string(&params->vstr, new_str);
                MEM_free(new_str);
            }
            crb_vstr_append_character(&params->vstr, L')');
            break;
        case CRB_SCOPE_CHAIN_VALUE: /* FALLTHRU*/
        default:
            sprintf(params->buf, "<%s>", CRB_get_type_name(value->type));
    }

    CRB_Char* ret = params->vstr.string;
    params->deep--;
    if (params->deep == 0) {
        close_rbtree(params->record);
        MEM_free(params);
    }
    return ret;
}
