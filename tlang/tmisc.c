//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include <wchar.h>
#include <limits.h>
#include <include/tlang.h>
#include "tinterpreter.h"
#include "tmisc.h"


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
        case INDEX_EXPRESSION:      /* FALLTHRU */
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
        case CRB_DOUBLE_VALUE:
            return "dobule";
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
static int my_strlen(CRB_Char* str) {
    if (str == NULL) {
        return 0;
    }
    return CRB_wcslen(str);
}

void crb_vstr_clear(VString *v) {
    v->string = NULL;
}

void crb_vstr_append_string(VString *v, const CRB_Char *str) {
    int old_len = my_strlen(v->string);
    int new_size = sizeof(CRB_Char) * (old_len + CRB_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    CRB_wcscpy(&v->string[old_len], str);
}

void crb_vstr_append_character(VString *v, CRB_Char ch) {
    int current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string,sizeof(CRB_Char)*(current_len+2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}




CRB_Value* CRB_add_global_variable(CRB_Interpreter *inter, const char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    Variable* new_var = crb_execute_malloc(inter, sizeof(Variable));
    new_var->is_final = is_final;
    char* name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(name, identifier);
    new_var->name = name;
    new_var->value = *value;
    rbtree_set(inter->global_vars, ptr_value(new_var));
//    new_var->next = inter->variable;
//    inter->variable = new_var;
    return &new_var->value;
}

Variable* crb_search_global_variable(CRB_Interpreter *inter, const char *identifier) {
    if (inter->global_vars == NULL) {
        printf("@@@@@@crb_search_global_variable\n");
        return NULL;
    }
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->global_vars, ptr_value(&key), NULL);
    return (Variable*)res.ptr_value;
}

CRB_Value* CRB_search_global_variable(CRB_Interpreter *inter, const char *identifier, CRB_Boolean *is_final) {
    if (inter->global_vars == NULL) {
        printf("@@@@@@CRB_search_global_variable\n");
        return NULL;
    }
    int ok;
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->global_vars, ptr_value(&key), &ok);
    if (ok) {
        Variable* var = (Variable*)res.ptr_value;
        if (is_final) {
            *is_final = var->is_final;
        }
        return &var->value;
    }
    return NULL;
//    Variable* pos;
//    for (pos=inter->variable; pos; pos = pos->next) {
//        if (!strcmp(pos->name, identifier))
//            break;
//    }
//    if (pos == NULL) {
//        return NULL;
//    } else {
//        return &pos->value;
//    }
}

CRB_Value* CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env, const char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->variable->type));
    CRB_Value* ret = CRB_add_assoc_member(inter, env->variable->u.scope_chain.frame, identifier, value, is_final);
    return ret;
}

CRB_Value* CRB_search_local_variable(CRB_LocalEnvironment *env, const char *identifier, CRB_Boolean *is_final) {
    if (env == NULL) {
        return NULL;
    }
    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->variable->type));

    for (CRB_Object* sc = env->variable; sc; sc = sc->u.scope_chain.next) {
        DBG_assert(sc->type == SCOPE_CHAIN_OBJECT, ("sc->type..%d\n", sc->type));
        CRB_Value* value = CRB_search_assoc_member(sc->u.scope_chain.frame, identifier, is_final);
        if (value) {
            return value;
        }
    }
    return NULL;
}

CRB_FunctionDefinition* crb_search_function_in_compile(const char *name) {
    CRB_Interpreter* inter = crb_get_current_interpreter();
    return CRB_search_function(inter, name);
}

CRB_FunctionDefinition* CRB_search_function(CRB_Interpreter *inter, const char *name) {
    NamedItemEntry key = {name};
    VALUE res = rbtree_get(inter->functions, ptr_value(&key), NULL);
    return (CRB_FunctionDefinition*)res.ptr_value;
//    for (CRB_FunctionDefinition *pos = inter->function_list; pos; pos = pos->next) {
//        if (!strcmp(pos->name, name))
//            break;
//    }
//    return pos;
}

void* CRB_object_get_native_pointer(CRB_Object *obj) {
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT, ("obj->type..%d\n", obj->type));
    return obj->u.native_pointer.pointer;
}

// char
int CRB_mbstowcs_len(const char *src) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int src_idx, dest_idx;
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        int status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
        if (status < 0) {
            return status;
        }
        dest_idx++;
        src_idx += status;
    }

    return dest_idx;
}

void CRB_mbstowcs(const char *src, CRB_Char *dest) {
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(&dest[dest_idx], &src[src_idx], MB_LEN_MAX, &ps);
        dest_idx++;
        src_idx += status;
    }
    dest[dest_idx] = L'\0';
}

CRB_Char* CRB_mbstowcs_alloc(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *src) {
    int len = CRB_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
//        crb_runtime_error(inter, env, line_number, BAD_MULTIBYTE_CHARACTER_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Char* ret = MEM_malloc(sizeof(CRB_Char)*(len+1));
    CRB_mbstowcs(src, ret);
    return ret;
}

int CRB_wcstombs_len(const CRB_Char *src) {
    char dummy[MB_LEN_MAX];
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int dest_idx;
    for (int src_idx = dest_idx = 0; src[src_idx] != L'\0'; ++src_idx) {
        int status = wcrtomb(dummy, src[src_idx], &ps);
        dest_idx += status;
    }
    return dest_idx;
}

void CRB_wcstombs(const CRB_Char *src, char *dest) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(mbstate_t));
    int dest_idx;
    for (int src_idx = dest_idx = 0; src[src_idx] != '\0'; ++src_idx) {
        int status = wcrtomb(&dest[dest_idx], src[src_idx], &ps);
        dest_idx += status;
    }
    dest[dest_idx] = '\0';
}

char CRB_wctochar(CRB_Char src) {
    mbstate_t ps;
    char dest;
    memset(&ps, 0, sizeof(mbstate_t));
    int status = wcrtomb(&dest, src, &ps);
    DBG_assert(status == 1, ("wcrtomb status..%d\n", status));
    return dest;
}

int CRB_print_wcs(FILE *fp, const CRB_Char *str) {
    return fwprintf(fp, str);
//    int mb_len = CRB_wcstombs_len(str);
//    char* tmp = MEM_malloc(mb_len + 1);
//    CRB_wcstombs(str, tmp);
//    int result = fprintf(fp, "%s", tmp);
//    MEM_free(tmp);
//    return result;
}

int CRB_print_wcs_ln(FILE *fp, const CRB_Char *str) {
    return fwprintf(fp, L"%s\n", str);
//    int result = CRB_print_wcs(fp, str);
//    fprintf(fp, "\n");
//    return result;
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
//        new_str = CRB_value_to_string(params->inter, params->env, params->line_number, &value->value);
//        crb_vstr_append_string(&params->params->vstr, new_str);
//        MEM_free(new_str);
    }
    return 0;
}

CRB_Char* CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const CRB_Value *value, void* param) {
    struct _value_to_string_params* params = (struct _value_to_string_params*)param;
    if (params == NULL) {
        params = (struct _value_to_string_params*)MEM_malloc(sizeof(struct _value_to_string_params));
        crb_vstr_clear(&params->vstr);
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
            sprintf(params->buf, "%d", value->u.int_value);
            CRB_mbstowcs(params->buf, params->wc_buf);
            crb_vstr_append_string(&params->vstr, params->wc_buf);
            break;
        case CRB_DOUBLE_VALUE:
            sprintf(params->buf, "%lf", value->u.double_value);
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
//                    CRB_Char* new_str = CRB_value_to_string(inter, env, line_number, data+i);
//                    crb_vstr_append_string(&params->vstr, new_str);
//                    MEM_free(new_str);
                }
            }
            crb_vstr_append_character(&params->vstr, L']');
            break;
        case CRB_ASSOC_VALUE:
            crb_vstr_append_character(&params->vstr, L'{');
            AssocMember_RBTREE* tr = value->u.object->u.assoc.members;
            struct _assoc_every_member_to_string_params _params = {CRB_FALSE, inter, env, line_number, params};
            AssocMember_rbtree_ldr(tr, _assoc_every_member_to_string, &_params);
//            for (i = 0; i < value->u.object->u.assoc.member_count; i++) {
//                if (i > 0) {
//                    CRB_mbstowcs(", ", wc_buf);
//                    crb_vstr_append_string(&vstr, wc_buf);
//                }
//                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.object->u.assoc.member[i].name);
//                DBG_assert(new_str != NULL, ("new_str is null.\n"));
//                crb_vstr_append_string(&vstr, new_str);
//                MEM_free(new_str);
//
//                CRB_mbstowcs("=>", wc_buf);
//                crb_vstr_append_string(&vstr, wc_buf);
//                new_str = CRB_value_to_string(inter, env, line_number, &value->u.object->u.assoc.member[i].value);
//                crb_vstr_append_string(&vstr, new_str);
//                MEM_free(new_str);
//            }
            crb_vstr_append_character(&params->vstr, L'}');
            break;
        case CRB_CLOSURE_VALUE:
            crb_vstr_append_string(&params->vstr, L"closure(");
            if (value->u.closure.function->name == NULL) {
                crb_vstr_append_string(&params->vstr, L"null");
            } else {
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.closure.function->name);
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
//            DBG_panic(("value->type..%d\n", value->type));
    }

    CRB_Char* ret = params->vstr.string;
    params->deep--;
    if (params->deep == 0) {
        close_rbtree(params->record);
        MEM_free(params);
    }
    return ret;
}
