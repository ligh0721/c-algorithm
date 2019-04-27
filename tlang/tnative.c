//
// Created by t5w0rd on 19-4-20.
//

#include <include/tlang.h>
#include <time.h>
#include "tinterpreter.h"
#include "tnative.h"
#include "tmisc.h"
#include "theap.h"
#include "terror.h"


CRB_Value CRB_Null_Value;
CRB_Value CRB_True_Value;
CRB_Value CRB_False_Value;

void crb_init_native_const_values() {
    CRB_Null_Value.type = CRB_NULL_VALUE;

    CRB_True_Value.type = CRB_BOOLEAN_VALUE;
    CRB_True_Value.u.boolean_value = CRB_TRUE;

    CRB_False_Value.type = CRB_BOOLEAN_VALUE;
    CRB_False_Value.u.boolean_value = CRB_FALSE;
}

typedef enum {
    FOPEN_ARGUMENT_TYPE_ERR = 0,
    FCLOSE_ARGUMENT_TYPE_ERR,
    FGETS_ARGUMENT_TYPE_ERR,
    FILE_ALREADY_CLOSED_ERR,
    FPUTS_ARGUMENT_TYPE_ERR,
    NEW_ARRAY_ARGUMENT_TYPE_ERR,
    NEW_ARRAY_ARGUMENT_TOO_FEW_ERR,
    EXIT_ARGUMENT_TYPE_ERR,
    NEW_EXCEPTION_ARGUMENT_ERR,
    FGETS_BAD_MULTIBYTE_CHARACTER_ERR
} NativeErrorCode;

extern CRB_ErrorDefinition crb_native_error_message_format[];

static CRB_NativeLibInfo st_lib_info = {crb_native_error_message_format};

// file pointers
static void file_finalizer(CRB_Interpreter *inter, CRB_Object *obj) {
    FILE* fp = (FILE*)CRB_object_get_native_pointer(obj);
    if (fp && fp != stdin && fp != stdout && fp != stderr) {
        fclose(fp);
    }
}

static const CRB_NativePointerInfo st_file_type_info = {
        "tlang.lang.file",
        file_finalizer
};

void crb_add_std_fp(CRB_Interpreter *inter) {
    CRB_Value fp_value;
    fp_value.type = CRB_NATIVE_POINTER_VALUE;
    fp_value.u.object = crb_create_native_pointer_i(inter, stdin, &st_file_type_info);
    CRB_add_global_variable(inter, "STDIN", &fp_value, CRB_TRUE);

    fp_value.u.object = crb_create_native_pointer_i(inter, stdout, &st_file_type_info);
    CRB_add_global_variable(inter, "STDOUT", &fp_value, CRB_TRUE);

    fp_value.u.object = crb_create_native_pointer_i(inter, stderr, &st_file_type_info);
    CRB_add_global_variable(inter, "STDERR", &fp_value, CRB_TRUE);
}

// object functions
CRB_Char* CRB_object_get_string(CRB_Object *obj) {
    DBG_assert(obj->type == STRING_OBJECT, ("obj->type..%d\n", obj->type));
    return obj->u.string.string;
}

void* CRB_object_get_native_pointer(CRB_Object *obj) {
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT, ("obj->type..%d\n", obj->type));
    return obj->u.native_pointer.pointer;
}

void CRB_object_set_native_pointer(CRB_Object *obj, void *p) {
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT, ("obj->type..%d\n", obj->type));
    obj->u.native_pointer.pointer = p;
}

// array functions
void CRB_array_set(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int index, CRB_Value *value) {
    DBG_assert(obj->type == ARRAY_OBJECT, ("obj->type..%d\n", obj->type));
    CRB_Value_slice_set(obj->u.array.array, index, *value);
}

struct _array_record {
    void* ref_array;
    long ref_count;
    long cap;
};

static inline void record_slice_state(CRB_Value_SLICE* sli, struct _array_record* record) {
    CRB_Value_ARRAY* arr = CRB_Value_slice_array_ref(sli);
    record->ref_array = arr;
    record->ref_count = CRB_Value_array_ref(arr);
    record->cap = CRB_Value_array_cap(arr);
}

static inline void check_slice_state(CRB_Value_SLICE* sli, const struct _array_record* record, Heap* heap) {
    CRB_Value_ARRAY* arr = CRB_Value_slice_array_ref(sli);
    if (arr == record->ref_array) {
        return;
    }
    if (record->ref_count == 1) {
        heap->current_heap_size -= sizeof(CRB_Value) * record->cap;
    }
    if (CRB_Value_array_ref(arr) == 1) {
        heap->current_heap_size += sizeof(CRB_Value) * CRB_Value_array_cap(arr);
    }
}

inline void CRB_array_append(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value *new_value) {
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    CRB_Value_SLICE* sli = obj->u.array.array;
    struct _array_record record;
    record_slice_state(sli, &record);
    CRB_Value_slice_append(sli, *new_value);
    check_slice_state(sli, &record, &inter->heap);
}

inline void CRB_array_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int pos, CRB_Value *new_value, int line_number) {
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    CRB_Value_SLICE* sli = obj->u.array.array;
    long len = CRB_Value_slice_len(sli);
    if (pos < 0 || pos > len) {
        crb_runtime_error(inter, env, line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", len, CRB_INT_MESSAGE_ARGUMENT, "index", pos, CRB_MESSAGE_ARGUMENT_END);
    }
    struct _array_record record;
    record_slice_state(sli, &record);
    CRB_Value_slice_insert(sli, pos, *new_value);
    check_slice_state(sli, &record, &inter->heap);
}

inline void CRB_array_pop(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int pos, int line_number, CRB_Value* popped) {
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    CRB_Value_SLICE* sli = obj->u.array.array;
    long len = CRB_Value_slice_len(sli);
    if (pos < 0 || pos >= len) {
        crb_runtime_error(inter, env, line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", len, CRB_INT_MESSAGE_ARGUMENT, "index", pos, CRB_MESSAGE_ARGUMENT_END);
    }
    if (popped != NULL) {
        *popped = CRB_Value_slice_pop(sli, pos);
    } else {
        CRB_Value_slice_pop(sli, pos);
    }
}

CRB_FunctionDefinition* CRB_add_native_function(CRB_Interpreter *inter, const char *name, int param_count, CRB_NativeFunctionFunc *func) {
    CRB_FunctionDefinition* fd = crb_malloc(sizeof(CRB_FunctionDefinition));
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINE;
    fd->is_closure = CRB_FALSE;
    fd->u.native_f.param_count = param_count;
    fd->u.native_f.func = func;
    rbtree_set(inter->functions, ptr_value(fd));
    return fd;
}

void CRB_set_function_definition(const char *name, int param_count, CRB_NativeFunctionFunc *func, CRB_FunctionDefinition *fd) {
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINE;
    fd->is_closure = CRB_TRUE;
    fd->u.native_f.param_count = param_count;
    fd->u.native_f.func = func;
}

void CRB_check_argument_count(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, int arg_count,
                              int expected_count) {
    if (expected_count < 0) {
        return;
    }
    if (arg_count < expected_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    } else if (arg_count > expected_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_MANY_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

FakeMethodDefinition* crb_add_fake_method(CRB_Interpreter *inter, ObjectType type, const char *name, int param_count, FakeMethodFunc *func) {
    FakeMethodDefinition* fd = crb_malloc(sizeof(FakeMethodDefinition));
    fd->type = type;
    fd->name = name;
    fd->param_count = param_count;
    fd->func = func;
    rbtree_set(inter->fake_methods, ptr_value(fd));
    return fd;
}

// functions
static void nv_print_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    for (int i=0; i<arg_count; ++i) {
        if (i > 0) {
            fprintf(stdout, " ");
        }
        CRB_Char* str = CRB_value_to_string(inter, env, __LINE__, args+i, NULL);
        CRB_print_wcs(stdout, str);
        MEM_free(str);
    }
    result->type = CRB_NULL_VALUE;
}

static void nv_println_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    for (int i=0; i<arg_count; ++i) {
        if (i > 0) {
            fprintf(stdout, " ");
        }
        CRB_Char* str = CRB_value_to_string(inter, env, __LINE__, args+i, NULL);
        CRB_print_wcs(stdout, str);
        MEM_free(str);
    }
    fprintf(stdout, "\n");
    result->type = CRB_NULL_VALUE;
}

static void nv_clock_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    result->type = CRB_INT_VALUE;
    result->u.int_value = clock();
}

static void nv_fopen_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    if (args[0].type != CRB_STRING_VALUE || args[1].type != CRB_STRING_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)FOPEN_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    char* filename = CRB_wcstombs_alloc(CRB_object_get_string(args[0].u.object));
    char* mode = CRB_wcstombs_alloc(CRB_object_get_string(args[1].u.object));
    FILE* fp = fopen(filename, mode);
    if (fp == NULL) {
        result->type = CRB_NULL_VALUE;
    } else {
        result->type = CRB_NATIVE_POINTER_VALUE;
        result->u.object = CRB_create_native_pointer(inter, env, fp, &st_file_type_info);
    }
    MEM_free(filename);
    MEM_free(mode);
}

static inline void check_file_pointer(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj) {
    FILE* fp = (FILE*)CRB_object_get_native_pointer(obj);
    if (fp == NULL) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)FILE_ALREADY_CLOSED_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

static void nv_fclose_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    if (args[0].type != CRB_NATIVE_POINTER_VALUE || (!CRB_check_native_pointer_type(args[0].u.object, &st_file_type_info))) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)FCLOSE_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    check_file_pointer(inter,env, args[0].u.object);
    FILE* fp = CRB_object_get_native_pointer(args[0].u.object);
    fclose(fp);
    CRB_object_set_native_pointer(args[0].u.object, NULL);
    result->type = CRB_NULL_VALUE;
}

static void nv_fgets_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    if (args[0].type != CRB_NATIVE_POINTER_VALUE || (!CRB_check_native_pointer_type(args[0].u.object, &st_file_type_info))) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)FGETS_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    check_file_pointer(inter,env, args[0].u.object);
    FILE* fp = CRB_object_get_native_pointer(args[0].u.object);
    int ret_len = 0;
    char* mb_buf = NULL;
    char buf[LINE_BUF_SIZE];
    while (fgets(buf, LINE_BUF_SIZE, fp)) {
        int new_len = ret_len + strlen(buf);
        mb_buf = MEM_realloc(mb_buf, new_len + 1);
        if (ret_len == 0) {
            strcpy(mb_buf, buf);
        } else {
            strcat(mb_buf, buf);
        }
        ret_len = new_len;
        if (mb_buf[ret_len-1] == '\n')
            break;
    }
    if (ret_len > 0) {
        CRB_Char* wc_str = CRB_mbstowcs_alloc(inter, env, __LINE__, mb_buf);
        if (wc_str == NULL) {
            MEM_free(mb_buf);
            CRB_error(inter, env, &st_lib_info, __LINE__, (int)FGETS_BAD_MULTIBYTE_CHARACTER_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        result->type = CRB_STRING_VALUE;
        result->u.object = CRB_create_crowbar_string(inter, env, wc_str);
    } else {
        result->type = CRB_NULL_VALUE;
    }
    MEM_free(mb_buf);
}

static void nv_fputs_func(CRB_Interpreter *inter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args, CRB_Value *result){
    if (args[0].type != CRB_STRING_VALUE || args[1].type != CRB_NATIVE_POINTER_VALUE || (!CRB_check_native_pointer_type(args[1].u.object, &st_file_type_info))) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)FPUTS_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    check_file_pointer(inter,env, args[1].u.object);
    FILE* fp = CRB_object_get_native_pointer(args[1].u.object);
    CRB_print_wcs(fp, CRB_object_get_string(args[0].u.object));
    result->type = CRB_NULL_VALUE;
}

static inline void new_array_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, int arg_idx, CRB_Value *result) {
    if (args[arg_idx].type != CRB_INT_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)NEW_ARRAY_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    int size = args[arg_idx].u.int_value;

    result->type = CRB_ARRAY_VALUE;
    result->u.object = CRB_create_array(inter, env, size);

    if (arg_idx == arg_count-1) {
        for (int i=0; i<size; ++i) {
            CRB_array_set(inter, env, result->u.object, i, &CRB_Null_Value);
        }
    } else {
        CRB_Value value;
        for (int i=0; i<size; ++i) {
            new_array_sub(inter, env, arg_count, args, arg_idx+1, &value);
            CRB_array_set(inter, env, result->u.object, i, &value);
        }
    }
}

static void nv_array_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    if (arg_count < 1) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)NEW_ARRAY_ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    new_array_sub(inter, env, arg_count, args, 0, result);
}

static void nv_object_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    result->type = CRB_ASSOC_VALUE;
    result->u.object = CRB_create_assoc(inter, env);
}

static void nv_exception_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    if (args[0].type != CRB_STRING_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)NEW_EXCEPTION_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(args[0].type), CRB_MESSAGE_ARGUMENT_END);
    }
    result->type = CRB_ASSOC_VALUE;
    result->u.object = CRB_create_exception(inter, env->next, args[0].u.object, env->caller_line_number);
}

static void nv_exit_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    if (args[0].type != CRB_INT_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)EXIT_ARGUMENT_TYPE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(args[0].type), CRB_MESSAGE_ARGUMENT_END);
    }
    exit(args[0].u.int_value);
    *result = CRB_Null_Value;
}

static void nv_str_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    CRB_Char* str = CRB_value_to_string(inter, env, __LINE__, args, NULL);
    result->type = CRB_STRING_VALUE;
    result->u.object = crb_create_crowbar_string_i(inter, str);
}

void crb_add_native_functions(CRB_Interpreter *inter) {
    CRB_add_native_function(inter, "print", -1, nv_print_func);
    CRB_add_native_function(inter, "println", -1, nv_println_func);
    CRB_add_native_function(inter, "clock", 0, nv_clock_func);
    CRB_add_native_function(inter, "fopen", 2, nv_fopen_func);
    CRB_add_native_function(inter, "fclose", 1, nv_fclose_func);
    CRB_add_native_function(inter, "fgets", 1, nv_fgets_func);
    CRB_add_native_function(inter, "fputs", 2, nv_fputs_func);
    CRB_add_native_function(inter, "array", -1, nv_array_func);
    CRB_add_native_function(inter, "object", 0, nv_object_func);
    CRB_add_native_function(inter, "exception", 1, nv_exception_func);
    CRB_add_native_function(inter, "exit", 1, nv_exit_func);
    CRB_add_native_function(inter, "str", 1, nv_str_func);
}

// fake methods
static void array_method_len(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    result->type = CRB_INT_VALUE;
    result->u.int_value = CRB_Value_slice_len(obj->u.array.array);
}

static void array_method_append(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    CRB_Value_SLICE* sli = obj->u.array.array;
    struct _array_record record;
    for (int i=0; i<arg_count; ++i) {
        record_slice_state(sli, &record);
        CRB_Value_slice_append(sli, args[i]);
        check_slice_state(sli, &record, &inter->heap);
    }
    result->type = CRB_NULL_VALUE;
}

static void array_method_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    CRB_Value* pos = &args[0];
    CRB_Value* new_value = &args[1];
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__, ARRAY_INSERT_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(pos->type), CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_insert(inter, env, obj, pos->u.int_value, new_value, __LINE__);
    result->type = CRB_NULL_VALUE;
}

static void array_method_pop(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    CRB_Value* pos = args;
    args->u.int_value = 0;
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__, ARRAY_REMOVE_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(pos->type), CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_pop(inter, env, obj, pos->u.int_value, __LINE__, result);
}

static void array_iterator_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    CRB_Value array;
    array.type = CRB_ARRAY_VALUE;
    array.u.object = obj;
    CRB_call_function_by_name(inter, env, __LINE__, ARRAY_ITERATOR_METHOD_NAME, 1, &array, result);
}

static void string_len_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result) {
    result->type = CRB_INT_VALUE;
    result->u.int_value = CRB_wcslen(obj->u.string.string);
}

void crb_add_fake_methods(CRB_Interpreter *inter) {
    crb_add_fake_method(inter, ARRAY_OBJECT, "len", 0, array_method_len);
    crb_add_fake_method(inter, ARRAY_OBJECT, "append", -1, array_method_append);
    crb_add_fake_method(inter, ARRAY_OBJECT, "insert", 2, array_method_insert);
    crb_add_fake_method(inter, ARRAY_OBJECT, "pop", 1, array_method_pop);
    crb_add_fake_method(inter, ARRAY_OBJECT, "iterator", 0, array_iterator_method);
    crb_add_fake_method(inter, STRING_OBJECT, "len", 0, string_len_method);
}
