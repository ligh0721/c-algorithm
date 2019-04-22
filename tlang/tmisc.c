//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "tmisc.h"


void* crb_malloc(size_t size) {
    CRB_Interpreter* inter = crb_get_current_interpreter();
    return MEM_storage_malloc(inter->interpreter_storage, size);
}

void* crb_execute_malloc(CRB_Interpreter *inter, size_t size) {
    return MEM_storage_malloc(inter->execute_storage, size);
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

void crb_vstr_append_string(VString *v, CRB_Char *str) {
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
    new_var->name = crb_execute_malloc(inter, strlen(identifier) + 1);
    strcpy(new_var->name, identifier);
    new_var->value = *value;
    rbtree_set(inter->variables, ptr_value(new_var));
//    new_var->next = inter->variable;
//    inter->variable = new_var;
    return &new_var->value;
}

Variable* crb_search_global_variable(CRB_Interpreter *inter, const char *identifier) {
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->variables, ptr_value(&key), NULL);
    return (Variable*)res.ptr_value;
}

CRB_Value* CRB_search_global_variable(CRB_Interpreter *inter, const char *identifier, CRB_Boolean *is_final) {
    int ok;
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->variables, ptr_value(&key), &ok);
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

CRB_Value* CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env, char *identifier, CRB_Value *value, CRB_Boolean is_final) {
    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->variable->type));
    CRB_Value* ret = CRB_add_assoc_member(inter, env->variable->u.scope_chain.frame, identifier, value, is_final);
    return ret;
}

CRB_Value* CRB_search_local_variable(CRB_LocalEnvironment *env, const char *identifier, CRB_Boolean *is_final) {
    if (env == NULL) {
        return NULL;
    }
    DBG_assert(env->variable->type == SCOPE_CHAIN_OBJECT, ("type..%d\n", env->variable->type));

    CRB_Value *value;
    for (CRB_Object* sc = env->variable; sc; sc = sc->u.scope_chain.next) {
        DBG_assert(sc->type == SCOPE_CHAIN_OBJECT, ("sc->type..%d\n", sc->type));
        value = CRB_search_assoc_member(sc->u.scope_chain.frame, identifier, is_final);
        if (value) {
            break;
        }
    }
    return value;
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
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    int src_idx, dest_idx;
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
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

int CRB_print_wcs(FILE *fp, CRB_Char *str) {
    int mb_len = CRB_wcstombs_len(str);
    char* tmp = MEM_malloc(mb_len + 1);
    CRB_wcstombs(str, tmp);
    int result = fprintf(fp, "%s", tmp);
    MEM_free(tmp);
    return result;
}

int CRB_print_wcs_ln(FILE *fp, CRB_Char *str) {
    int result = CRB_print_wcs(fp, str);
    fprintf(fp, "\n");
    return result;
}

CRB_Char* CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *value) {
    VString     vstr;
    char        buf[LINE_BUF_SIZE];
    CRB_Char    wc_buf[LINE_BUF_SIZE];
    int         i;

    crb_vstr_clear(&vstr);

    switch (value->type) {
        case CRB_BOOLEAN_VALUE:
            if (value->u.boolean_value) {
                CRB_mbstowcs("true", wc_buf);
            } else {
                CRB_mbstowcs("false", wc_buf);
            }
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_INT_VALUE:
            sprintf(buf, "%d", value->u.int_value);
            CRB_mbstowcs(buf, wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_DOUBLE_VALUE:
            sprintf(buf, "%f", value->u.double_value);
            CRB_mbstowcs(buf, wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_STRING_VALUE:
            crb_vstr_append_string(&vstr, value->u.object->u.string.string);
            break;
        case CRB_NATIVE_POINTER_VALUE:
            sprintf(buf, "%s(%p)", value->u.object->u.native_pointer.info->name, value->u.object->u.native_pointer.pointer);
            CRB_mbstowcs(buf, wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_NULL_VALUE:
            CRB_mbstowcs("null", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_ARRAY_VALUE:
            CRB_mbstowcs("(", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            for (i = 0; i < value->u.object->u.array.size; i++) {
                CRB_Char *new_str;
                if (i > 0) {
                    CRB_mbstowcs(", ", wc_buf);
                    crb_vstr_append_string(&vstr, wc_buf);
                }
                new_str = CRB_value_to_string(inter, env, line_number, &value->u.object->u.array.array[i]);
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);
            }
            CRB_mbstowcs(")", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_ASSOC_VALUE:
            CRB_mbstowcs("(", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            for (i = 0; i < value->u.object->u.assoc.member_count; i++) {
                if (i > 0) {
                    CRB_mbstowcs(", ", wc_buf);
                    crb_vstr_append_string(&vstr, wc_buf);
                }
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.object->u.assoc.member[i].name);
                DBG_assert(new_str != NULL, ("new_str is null.\n"));
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);

                CRB_mbstowcs("=>", wc_buf);
                crb_vstr_append_string(&vstr, wc_buf);
                new_str = CRB_value_to_string(inter, env, line_number, &value->u.object->u.assoc.member[i].value);
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);
            }
            CRB_mbstowcs(")", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_CLOSURE_VALUE:
            CRB_mbstowcs("closure(", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            if (value->u.closure.function->name == NULL) {
                CRB_mbstowcs("null", wc_buf);
                crb_vstr_append_string(&vstr, wc_buf);
            } else {
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.closure.function->name);
                DBG_assert(new_str != NULL, ("new_str is null.\n"));
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);
            }
            CRB_mbstowcs(")", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_FAKE_METHOD_VALUE:
            CRB_mbstowcs("fake_method(", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            {
                CRB_Char* new_str = CRB_mbstowcs_alloc(inter, env, line_number, value->u.fake_method.method_name);
                DBG_assert(new_str != NULL, ("new_str is null.\n"));
                crb_vstr_append_string(&vstr, new_str);
                MEM_free(new_str);
            }
            CRB_mbstowcs(")", wc_buf);
            crb_vstr_append_string(&vstr, wc_buf);
            break;
        case CRB_SCOPE_CHAIN_VALUE: /* FALLTHRU*/
        default:
            DBG_panic(("value->type..%d\n", value->type));
    }

    return vstr.string;
}
