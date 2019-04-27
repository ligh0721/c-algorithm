//
// Created by t5w0rd on 19-4-20.
//

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

// functions
static CRB_Value nv_print_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    for (int i=0; i<arg_count; ++i) {
        if (i > 0) {
            fprintf(stdout, " ");
        }
        CRB_Char* str = CRB_value_to_string(interpreter, env, __LINE__, args+i, NULL);
        CRB_print_wcs(stdout, str);
        MEM_free(str);
    }
    return CRB_Null_Value;
}

static CRB_Value nv_println_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    for (int i=0; i<arg_count; ++i) {
        if (i > 0) {
            fprintf(stdout, " ");
        }
        CRB_Char* str = CRB_value_to_string(interpreter, env, __LINE__, args+i, NULL);
        CRB_print_wcs(stdout, str);
        MEM_free(str);
    }
    fprintf(stdout, "\n");
    return CRB_Null_Value;
}

static CRB_Value nv_fopen_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    CRB_Value value = {};
    return value;
}

static CRB_Value nv_fclose_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    CRB_Value value = {};
    return value;
}

static CRB_Value nv_fgets_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    CRB_Value value = {};
    return value;
}

static CRB_Value nv_fputs_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment* env, int arg_count, CRB_Value *args){
    CRB_Value value = {};
    return value;
}

static inline CRB_Value new_array_sub(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, int arg_idx) {
    if (args[arg_idx].type != CRB_INT_VALUE) {
        CRB_error(inter, env, &st_lib_info, __LINE__, (int)NEW_ARRAY_ARGUMENT_TYPE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }

    int size = args[arg_idx].u.int_value;
    CRB_Value ret;
    ret.type = CRB_ARRAY_VALUE;
    ret.u.object = CRB_create_array(inter, env, size);

    if (arg_idx == arg_count-1) {
        for (int i=0; i<size; ++i) {
            CRB_array_set(inter, env, ret.u.object, i, &CRB_Null_Value);
        }
    } else {
        for (int i=0; i<size; ++i) {
            CRB_Value value = new_array_sub(inter, env, arg_count, args, arg_idx+1);
            CRB_array_set(inter, env, ret.u.object, i, &value);
        }
    }
    return ret;
}

static CRB_Value nv_array_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args) {
    if (arg_count < 1) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__, (int)NEW_ARRAY_ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    return new_array_sub(interpreter, env, arg_count, args, 0);
}

static CRB_Value nv_object_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args) {
    CRB_check_argument_count(interpreter, env, arg_count, 0);
    CRB_Value value;
    value.type = CRB_ASSOC_VALUE;
    value.u.object = CRB_create_assoc(interpreter, env);
    return value;
}

static CRB_Value nv_exception_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args) {
    CRB_check_argument_count(interpreter, env, arg_count, 1);
    if (args[0].type != CRB_STRING_VALUE) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__, (int)NEW_EXCEPTION_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(args[0].type), CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Value value;
    value.type = CRB_ASSOC_VALUE;
    value.u.object = CRB_create_exception(interpreter, env->next, args[0].u.object, env->caller_line_number);
    return value;
}

static CRB_Value nv_exit_proc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args) {
    CRB_check_argument_count(interpreter, env, arg_count, 1);
    if (args[0].type != CRB_INT_VALUE) {
        CRB_error(interpreter, env, &st_lib_info, __LINE__, (int)EXIT_ARGUMENT_TYPE_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(args[0].type), CRB_MESSAGE_ARGUMENT_END);
    }
    exit(args[0].u.int_value);
    return CRB_Null_Value;
}

CRB_FunctionDefinition* CRB_add_native_function(CRB_Interpreter *interpreter, const char *name, CRB_NativeFunctionProc *proc) {
    CRB_FunctionDefinition* fd = crb_malloc(sizeof(CRB_FunctionDefinition));
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINE;
    fd->is_closure = CRB_FALSE;
    fd->u.native_f.proc = proc;
    rbtree_set(interpreter->functions, ptr_value(fd));
    return fd;
}

CRB_Value CRB_create_closure(CRB_LocalEnvironment *env, CRB_FunctionDefinition *fd) {
    CRB_Value ret;
    ret.type = CRB_CLOSURE_VALUE;
    ret.u.closure.function = fd;
    ret.u.closure.environment = env->variable;
    return ret;
}

void crb_add_native_functions(CRB_Interpreter *inter) {
    CRB_add_native_function(inter, "print", nv_print_proc);
    CRB_add_native_function(inter, "println", nv_println_proc);
    CRB_add_native_function(inter, "fopen", nv_fopen_proc);
    CRB_add_native_function(inter, "fclose", nv_fclose_proc);
    CRB_add_native_function(inter, "fgets", nv_fgets_proc);
    CRB_add_native_function(inter, "fputs", nv_fputs_proc);
    CRB_add_native_function(inter, "array", nv_array_proc);
    CRB_add_native_function(inter, "object", nv_object_proc);
    CRB_add_native_function(inter, "exception", nv_exception_proc);
    CRB_add_native_function(inter, "exit", nv_exit_proc);
}

// file pointers
static void file_finalizer(CRB_Interpreter *inter, CRB_Object *obj) {
    FILE* fp = (FILE*)CRB_object_get_native_pointer(obj);
    if (fp && fp != stdin && fp != stdout && fp != stderr) {
        fclose(fp);
    }
}

static CRB_NativePointerInfo st_file_type_info = {
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

void CRB_set_function_definition(const char *name, CRB_NativeFunctionProc *proc, CRB_FunctionDefinition *fd) {
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINE;
    fd->is_closure = CRB_TRUE;
    fd->u.native_f.proc = proc;
}

// fake method
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

inline CRB_Value CRB_array_pop(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int pos, int line_number) {
    DBG_assert(obj->type == ARRAY_OBJECT, ("bad type..%d\n", obj->type));
    CRB_Value_SLICE* sli = obj->u.array.array;
    long len = CRB_Value_slice_len(sli);
    if (pos < 0 || pos >= len) {
        crb_runtime_error(inter, env, line_number, ARRAY_INDEX_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "size", len, CRB_INT_MESSAGE_ARGUMENT, "index", pos, CRB_MESSAGE_ARGUMENT_END);
    }
    return CRB_Value_slice_pop(sli, pos);
}

static void array_method_len(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    result->type = CRB_INT_VALUE;
    result->u.int_value = CRB_Value_slice_len(obj->u.array.array);
}

static void array_method_append(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    CRB_Value_SLICE* sli = obj->u.array.array;
    struct _array_record record;
    for (int i=arg_count-1; i>=0; --i) {
        record_slice_state(sli, &record);
        CRB_Value* new_value = CRB_peek_stack(inter, i);
        CRB_Value_slice_append(sli, *new_value);
        check_slice_state(sli, &record, &inter->heap);
    }
    result->type = CRB_NULL_VALUE;
}

static void array_method_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    CRB_Value* new_value = CRB_peek_stack(inter, 0);
    CRB_Value* pos = CRB_peek_stack(inter, 1);
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__, ARRAY_INSERT_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(pos->type), CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_array_insert(inter, env, obj, pos->u.int_value, new_value, __LINE__);
    result->type = CRB_NULL_VALUE;
}

static void array_method_pop(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    CRB_Value* pos = CRB_peek_stack(inter, 0);
    if (pos->type != CRB_INT_VALUE) {
        crb_runtime_error(inter, env, __LINE__, ARRAY_REMOVE_ARGUMENT_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(pos->type), CRB_MESSAGE_ARGUMENT_END);
    }
    *result = CRB_array_pop(inter, env, obj, pos->u.int_value, __LINE__);
}

static void array_iterator_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    CRB_Value array;
    array.type = CRB_ARRAY_VALUE;
    array.u.object = obj;
    *result = CRB_call_function_by_name(inter, env, __LINE__, ARRAY_ITERATOR_METHOD_NAME, 1, &array);
}

static void string_len_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value *result) {
    result->type = CRB_INT_VALUE;
    result->u.int_value = CRB_wcslen(obj->u.string.string);
}

FakeMethodTable st_fake_method_table[] = {
        {ARRAY_OBJECT, "len", 0, array_method_len},
        {ARRAY_OBJECT, "append", -1, array_method_append},
        {ARRAY_OBJECT, "insert", 2, array_method_insert},
        {ARRAY_OBJECT, "pop", 1, array_method_pop},
        {ARRAY_OBJECT, "iterator", 0, array_iterator_method},
        {STRING_OBJECT, "len", 0, string_len_method},
};

#define FAKE_METHOD_TABLE_SIZE(array) (sizeof(array) / sizeof((array)[0]))

FakeMethodTable* crb_search_fake_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_FakeMethod *fm) {
    int i;
    for (i = 0; i < FAKE_METHOD_TABLE_SIZE(st_fake_method_table); i++) {
        if (fm->object->type == st_fake_method_table[i].type && !strcmp(fm->method_name, st_fake_method_table[i].name)) {
            break;
        }
    }
    if (i == FAKE_METHOD_TABLE_SIZE(st_fake_method_table)) {
        crb_runtime_error(inter, env, line_number, NO_SUCH_METHOD_ERR, CRB_STRING_MESSAGE_ARGUMENT, "method_name", fm->method_name, CRB_MESSAGE_ARGUMENT_END);
    }

    return &st_fake_method_table[i];
}
