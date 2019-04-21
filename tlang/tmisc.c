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

CRB_Value* CRB_add_global_variable(CRB_Interpreter *inter, char *identifier, CRB_Value *value, CRB_Boolean is_final) {
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

Variable* crb_search_global_variable(CRB_Interpreter *inter, char *identifier) {
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->variables, ptr_value(&key), NULL);
    return (Variable*)res.ptr_value;
}

CRB_Value* CRB_search_global_variable(CRB_Interpreter *inter, char *identifier) {
    int ok;
    NamedItemEntry key = {identifier};
    VALUE res = rbtree_get(inter->variables, ptr_value(&key), &ok);
    if (ok) {
        return &((Variable*)res.ptr_value)->value;
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