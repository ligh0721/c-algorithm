//
// Created by t5w0rd on 19-4-20.
//

#include <wchar.h>
#include <string.h>
#include "tinterpreter.h"
#include "theap.h"
#include "terror.h"
#include "tmisc.h"
#include "tnative.h"


static void check_gc(CRB_Interpreter *inter) {
#if 0
    crb_garbage_collect(inter);
#endif
    if (inter->heap.current_heap_size > inter->heap.current_threshold) {
//        fprintf(stderr, "garbage collecting...");
        crb_garbage_collect(inter);
//        fprintf(stderr, "done.\n");

        inter->heap.current_threshold = inter->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

// object
static CRB_Object* alloc_object(CRB_Interpreter *inter, ObjectType type) {
    check_gc(inter);
    CRB_Object* ret = MEM_malloc(sizeof(CRB_Object));
    inter->heap.current_heap_size += sizeof(CRB_Object);
    ret->type = type;
    ret->marked = CRB_FALSE;
    ret->prev = NULL;
    ret->next = inter->heap.header;
    inter->heap.header = ret;
    if (ret->next) {
        ret->next->prev = ret;
    }
    return ret;
}

// string object
static void add_ref_in_native_method(CRB_LocalEnvironment *env, CRB_Object *obj) {
    RefInNativeFunc* new_ref = MEM_malloc(sizeof(RefInNativeFunc));
    new_ref->object = obj;
    new_ref->next = env->ref_in_native_method;
    env->ref_in_native_method = new_ref;
}

CRB_Object* crb_literal_to_crb_string_i(CRB_Interpreter *inter, CRB_Char *str) {
    CRB_Object* ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    ret->u.string.is_literal = CRB_TRUE;
    return ret;
}

CRB_Object* crb_create_crowbar_string_i(CRB_Interpreter *inter, CRB_Char *str) {
    CRB_Object* ret = alloc_object(inter, STRING_OBJECT);
    ret->u.string.string = str;
    inter->heap.current_heap_size += sizeof(CRB_Char) * (CRB_wcslen(str) + 1);
    ret->u.string.is_literal = CRB_FALSE;
    return ret;
}

CRB_Object* CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Char *str) {
    CRB_Object* ret = crb_create_crowbar_string_i(inter, str);
    add_ref_in_native_method(env, ret);
    return ret;
}

CRB_Object* crb_string_substr_i(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *str, int from, int len, int line_number) {
    int org_len = CRB_wcslen(str->u.string.string);
    if (from < 0 || from >= org_len) {
        crb_runtime_error(inter, env, line_number, STRING_POS_OUT_OF_BOUNDS_ERR, CRB_INT_MESSAGE_ARGUMENT, "len", org_len, CRB_INT_MESSAGE_ARGUMENT, "pos", from, CRB_MESSAGE_ARGUMENT_END);
    }
    if (len < 0 || from + len > org_len) {
        crb_runtime_error(inter, env, line_number, STRING_SUBSTR_LEN_ERR, CRB_INT_MESSAGE_ARGUMENT, "len", from + len - org_len, CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_Char* new_str = MEM_malloc(sizeof(CRB_Char) * (len+1));
    CRB_wcsncpy(new_str, str->u.string.string + from, len);
    new_str[len] = L'\0';

    return crb_create_crowbar_string_i(inter, new_str);
}

// array object
CRB_Object* crb_create_array_i(CRB_Interpreter *inter, int size) {
    CRB_Object* ret = alloc_object(inter, ARRAY_OBJECT);
    ret->u.array.array = open_CRB_Value_slice(size, size);
    CRB_Value* data = CRB_Value_slice_data(ret->u.array.array);
    for (long i=0; i<size; ++i) {
        data[i].type = CRB_NULL_VALUE;
    }
    inter->heap.current_heap_size += sizeof(CRB_Value) * size;
    return ret;
}

CRB_Object* crb_create_array_slice_i(CRB_Interpreter *inter, CRB_Value_SLICE* arr, long begin, long end) {
    CRB_Object* ret = alloc_object(inter, ARRAY_OBJECT);
    ret->u.array.array = open_CRB_Value_slice_by_slice(arr, begin, end);
    return ret;
}

CRB_Object* CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int size) {
    CRB_Object* ret = crb_create_array_i(inter, size);
    add_ref_in_native_method(env, ret);
    return ret;
}

// assoc object
static int _AssocMember_compare(AssocMember a, AssocMember b) {
    return strcmp(a.name, b.name);
}

CRB_Object* crb_create_assoc_i(CRB_Interpreter *inter) {
    CRB_Object* ret = alloc_object(inter, ASSOC_OBJECT);
    ret->u.assoc.members = open_AssocMember_rbtree(_AssocMember_compare);
    return ret;
}

CRB_Object* CRB_create_assoc(CRB_Interpreter *inter, CRB_LocalEnvironment *env) {
    CRB_Object* ret = crb_create_assoc_i(inter);
    add_ref_in_native_method(env, ret);
    return ret;
}

CRB_Value* CRB_add_assoc_member(CRB_Interpreter *inter, CRB_Object *assoc, const char *name, CRB_Value *value, CRB_Boolean is_final) {
    check_gc(inter);
    AssocMember_RBTREE* tr = assoc->u.assoc.members;
    AssocMember key = {name, *value, is_final};
    AssocMember_RBNODE* parent;
    AssocMember_RBNODE** where = AssocMember_rbtree_fast_get(tr, key, &parent);
    if (!AssocMember_rbtree_node_not_found(tr, where)) {
        AssocMember* value_entry = AssocMember_rbtree_fast_value(tr, where);
        *value_entry = key;
        return &value_entry->value;
    }
    AssocMember_RBNODE* node = AssocMember_rbtree_open_node(tr, key, parent);
    AssocMember_rbtree_fast_set(tr, where, node);
    inter->heap.current_heap_size += sizeof(AssocMember);
    AssocMember* value_entry = AssocMember_rbtree_fast_value(tr, where);
    return &value_entry->value;
}

CRB_Value* CRB_search_assoc_member(CRB_Object *assoc, const char *member_name, CRB_Boolean* is_final) {
    AssocMember_RBTREE* tr = assoc->u.assoc.members;
    AssocMember key = {member_name};
    AssocMember_RBNODE** where = AssocMember_rbtree_fast_get(tr, key, NULL);
    if (AssocMember_rbtree_node_not_found(tr, where)) {
        return NULL;
    }
    AssocMember* value_entry = AssocMember_rbtree_fast_value(tr, where);
    if (is_final) {
        *is_final = value_entry->is_final;
    }
    return &value_entry->value;
}

// scope chain
CRB_Object* crb_create_scope_chain(CRB_Interpreter *inter) {
    CRB_Object *ret = alloc_object(inter, SCOPE_CHAIN_OBJECT);
    ret->u.scope_chain.frame = NULL;
    ret->u.scope_chain.next = NULL;
    return ret;
}

// native pointer
CRB_Object* crb_create_native_pointer_i(CRB_Interpreter *inter, void *pointer, const CRB_NativePointerInfo *info) {
    CRB_Object* ret = alloc_object(inter, NATIVE_POINTER_OBJECT);
    ret->u.native_pointer.pointer = pointer;
    ret->u.native_pointer.info = info;
    return ret;
}

CRB_Object* CRB_create_native_pointer(CRB_Interpreter *inter, CRB_LocalEnvironment *env, void *pointer, const CRB_NativePointerInfo *info) {
    CRB_Object* ret = crb_create_native_pointer_i(inter, pointer, info);
    add_ref_in_native_method(env, ret);
    return ret;
}

const CRB_NativePointerInfo* CRB_get_native_pointer_type(CRB_Object *native_pointer) {
    return native_pointer->u.native_pointer.info;
}


CRB_Boolean CRB_check_native_pointer_type(CRB_Object *native_pointer, const CRB_NativePointerInfo *info) {
    return native_pointer->u.native_pointer.info == info;
}

// exception
static int count_stack_trace_depth(CRB_LocalEnvironment *top) {
    int count = 1;
    for (CRB_LocalEnvironment*pos=top; pos!=NULL; pos=pos->next) {
        ++count;
    }
    return count;
}

static CRB_Object* create_stack_trace_line(CRB_Interpreter *inter, CRB_LocalEnvironment *env, const char *func_name, int line_number) {
    int stack_count = 0;

    CRB_Object* new_line = crb_create_assoc_i(inter);
    CRB_Value new_line_value;
    new_line_value.type = CRB_ASSOC_VALUE;
    new_line_value.u.object = new_line;
    CRB_push_value(inter, &new_line_value);
    stack_count++;

    CRB_Char* wc_func_name = CRB_mbstowcs_alloc(inter, env, line_number, func_name);
    DBG_assert(wc_func_name != NULL, ("wc_func_name is null.\n"));

    CRB_Value value;
    value.type = CRB_STRING_VALUE;
    value.u.object = crb_create_crowbar_string_i(inter, wc_func_name);
    CRB_push_value(inter, &value);
    stack_count++;

    CRB_add_assoc_member(inter, new_line, EXCEPTION_MEMBER_FUNCTION_NAME, &value, CRB_TRUE);

    value.type = CRB_INT_VALUE;
    value.u.int_value = line_number;
    CRB_add_assoc_member(inter, new_line, EXCEPTION_MEMBER_LINE_NUMBER, &value, CRB_TRUE);

    CRB_shrink_stack(inter, stack_count);

    return new_line;
}

static void print_stack_trace(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result) {
    CRB_Value* message = CRB_search_local_variable(env, EXCEPTION_MEMBER_MESSAGE, NULL);
    if (message == NULL) {
        crb_runtime_error(inter, env, __LINE__, EXCEPTION_HAS_NO_MESSAGE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    if (message->type != CRB_STRING_VALUE) {
        crb_runtime_error(inter, env, __LINE__, EXCEPTION_MESSAGE_IS_NOT_STRING_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(message->type), CRB_MESSAGE_ARGUMENT_END);
    }
    CRB_print_wcs_ln(stderr, message->u.object->u.string.string);

    CRB_Value* stack_trace = CRB_search_local_variable(env, EXCEPTION_MEMBER_STACK_TRACE, NULL);
    if (stack_trace == NULL) {
        crb_runtime_error(inter, env, __LINE__, EXCEPTION_HAS_NO_STACK_TRACE_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
    if (stack_trace->type != CRB_ARRAY_VALUE) {
        crb_runtime_error(inter, env, __LINE__, STACK_TRACE_IS_NOT_ARRAY_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(stack_trace->type), CRB_MESSAGE_ARGUMENT_END);
    }

    CRB_Value_SLICE* stack_trace_arr = stack_trace->u.object->u.array.array;
    CRB_Value* stack_trace_data = CRB_Value_slice_data(stack_trace_arr);
    long len = CRB_Value_slice_len(stack_trace_arr);
    for (long i=0; i<len; ++i) {
        if (stack_trace_data[i].type != CRB_ASSOC_VALUE) {
            crb_runtime_error(inter, env, __LINE__, STACK_TRACE_LINE_IS_NOT_ASSOC_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_Object* assoc = stack_trace_data[i].u.object;
        CRB_Value* line_number = CRB_search_assoc_member(assoc, EXCEPTION_MEMBER_LINE_NUMBER, NULL);
        if (line_number == NULL || line_number->type != CRB_INT_VALUE) {
            crb_runtime_error(inter, env, __LINE__, STACK_TRACE_LINE_HAS_NO_LINE_NUMBER_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_Value* function_name = CRB_search_assoc_member(assoc, EXCEPTION_MEMBER_FUNCTION_NAME, NULL);
        if (function_name == NULL || function_name->type != CRB_STRING_VALUE) {
            crb_runtime_error(inter, env, __LINE__, STACK_TRACE_LINE_HAS_NO_FUNC_NAME_ERR, CRB_MESSAGE_ARGUMENT_END);
        }

        // print line number
        fprintf(stderr, "module %s, line ", CRB_env_module(inter, env)->name);
        CRB_Char* str = CRB_value_to_string(inter, NULL, line_number->u.int_value, line_number, NULL);
        CRB_print_wcs(stderr, str);
        MEM_free(str);
        fprintf(stderr, ", in ");

        // print function name
        str = CRB_value_to_string(inter, NULL, line_number->u.int_value, function_name, NULL);
        CRB_print_wcs_ln(stderr, str);
        MEM_free(str);
    }
    result->type = CRB_NULL_VALUE;
}

/*
 * 创建exception对象，message时字符串对象
 */
CRB_Object* CRB_create_exception(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *message, int line_number) {
    static CRB_FunctionDefinition print_stack_trace_fd;

    int stack_count = 0;

    CRB_Object* ret = crb_create_assoc_i(inter);
    CRB_Value value;
    value.type = CRB_ASSOC_VALUE;
    value.u.object = ret;
    CRB_push_value(inter, &value);
    ++stack_count;

    value.type = CRB_STRING_VALUE;
    value.u.object = message;
    CRB_push_value(inter, &value);
    ++stack_count;
    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_MESSAGE, &value, CRB_TRUE);  // 添加.message(string)

    int stack_trace_depth = count_stack_trace_depth(env);
    CRB_Object* stack_trace = crb_create_array_i(inter, stack_trace_depth);
    value.type = CRB_ARRAY_VALUE;
    value.u.object = stack_trace;
    CRB_push_value(inter, &value);
    ++stack_count;
    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_STACK_TRACE, &value, CRB_TRUE);  // 添加.stack_trace(array)

    CRB_Object* line;  /* CRB_Assoc */
    int next_line_number = line_number;
    CRB_LocalEnvironment* env_pos;
    int stack_trace_idx;
    for (env_pos=env, stack_trace_idx=0; env_pos; env_pos=env_pos->next, ++stack_trace_idx) {
        const char *func_name;
        if (env_pos->current_function_name) {
            func_name = env_pos->current_function_name;
        } else {
            func_name = "anonymous closure";
        }
        line = create_stack_trace_line(inter, env, func_name, next_line_number);
        value.type = CRB_ASSOC_VALUE;
        value.u.object = line;
        CRB_array_set(inter, env, stack_trace, stack_trace_idx, &value);
        next_line_number = env_pos->caller_line_number;
    }

    line = create_stack_trace_line(inter, env, EXCEPTION_TRACE_TOP_LEVEL, next_line_number);
    value.type = CRB_ASSOC_VALUE;
    value.u.object = line;
    CRB_array_set(inter, env, stack_trace, stack_trace_idx, &value);

    CRB_set_native_function(CRB_env_module(inter, env), NULL, 0, print_stack_trace, &print_stack_trace_fd);

    CRB_push_value(inter, &value);
    ++stack_count;

    value.type = CRB_CLOSURE_VALUE;
    value.u.closure.function_definition = &print_stack_trace_fd;
    value.u.closure.scope_chain = NULL; /* to stop marking by GC */

    CRB_Value scope_chain;
    scope_chain.type = CRB_SCOPE_CHAIN_VALUE;
    scope_chain.u.object = crb_create_scope_chain(inter);
    scope_chain.u.object->u.scope_chain.frame = ret;
    CRB_push_value(inter, &scope_chain);
    ++stack_count;

    value.u.closure.scope_chain = scope_chain.u.object;

    CRB_add_assoc_member(inter, ret, EXCEPTION_MEMBER_PRINT_STACK_TRACE, &value, CRB_TRUE);  // 添加.print_stack_trace(native function)

    CRB_shrink_stack(inter, stack_count);

    return ret;
}

// gc
static void gc_mark_value(CRB_Value *v);

static int _gc_mark_every_assoc_member(const AssocMember *value, void *param) {
    gc_mark_value(&((AssocMember*)value)->value);
    return 0;
}

static void gc_mark(CRB_Object *obj) {
    if (obj == NULL) {
        return;
    }
    if (obj->marked) {
        return;
    }

    obj->marked = CRB_TRUE;

    if (obj->type == ARRAY_OBJECT) {
        CRB_Value_SLICE* arr = obj->u.array.array;
        long len = CRB_Value_slice_len(arr);
        CRB_Value* data = CRB_Value_slice_data(arr);
        for (long i=0; i<len; ++i) {
            gc_mark_value(data++);
        }
    } else if (obj->type == ASSOC_OBJECT) {
        AssocMember_RBTREE* tr = obj->u.assoc.members;
        AssocMember_rbtree_ldr(tr, _gc_mark_every_assoc_member, NULL);
    } else if (obj->type == SCOPE_CHAIN_OBJECT) {
        gc_mark(obj->u.scope_chain.frame);
        gc_mark(obj->u.scope_chain.next);
    }
}

static void gc_reset_mark(CRB_Object *obj) {
    obj->marked = CRB_FALSE;
}

static void gc_mark_value(CRB_Value *v) {
    if (crb_is_object_value(v->type)) {
        gc_mark(v->u.object);
    } else if (v->type == CRB_CLOSURE_VALUE) {
        if (v->u.closure.scope_chain) {
            gc_mark(v->u.closure.scope_chain);
        }
    } else if (v->type == CRB_FAKE_METHOD_VALUE) {
        gc_mark(v->u.fake_method.object);
    }
}

static void gc_mark_ref_in_native_method(CRB_LocalEnvironment *env) {
    for (RefInNativeFunc *ref = env->ref_in_native_method; ref; ref = ref->next) {
        gc_mark(ref->object);
    }
}

static int _every_variable_gc_mark_value(const VALUE* value, void* param) {
    Variable* v = (Variable*)value->ptr_value;
    gc_mark_value(&v->value);
    return 0;
}

static int _every_module(const VALUE* value, void* param) {
    CRB_Module* module = (CRB_Module*)value->ptr_value;
    if (module->global_vars != NULL) {
        rbtree_ldr(module->global_vars, _every_variable_gc_mark_value, NULL);
    }
    return 0;
}

static void gc_mark_objects(CRB_Interpreter *inter) {
    for (CRB_Object* obj = inter->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }

    if (inter->global_vars != NULL) {
        rbtree_ldr(inter->global_vars, _every_variable_gc_mark_value, NULL);
    }

    rbtree_ldr(inter->modules, _every_module, NULL);

    for (CRB_LocalEnvironment* lv=inter->top_environment; lv; lv=lv->next) {
        gc_mark(lv->scope_chain);
        gc_mark_ref_in_native_method(lv);
    }

    for (int i=0; i<inter->stack.stack_pointer; ++i) {
        gc_mark_value(&inter->stack.stack[i]);
    }

    gc_mark_value(&inter->current_exception);
}

static void gc_dispose_object(CRB_Interpreter *inter, CRB_Object *obj) {
    switch (obj->type) {
        case ARRAY_OBJECT:
            if (CRB_Value_array_ref(CRB_Value_slice_array_ref(obj->u.array.array)) == 1) {
                inter->heap.current_heap_size -= sizeof(CRB_Value) * CRB_Value_slice_cap(obj->u.array.array);
            }
            close_CRB_Value_slice(obj->u.array.array);
            break;
        case STRING_OBJECT:
            if (!obj->u.string.is_literal) {
                inter->heap.current_heap_size -= sizeof(CRB_Char) * (CRB_wcslen(obj->u.string.string) + 1);
                MEM_free(obj->u.string.string);
            }
            break;
        case ASSOC_OBJECT:
            inter->heap.current_heap_size -= sizeof(AssocMember) * AssocMember_rbtree_len(obj->u.assoc.members);
            close_AssocMember_rbtree(obj->u.assoc.members);
            break;
        case SCOPE_CHAIN_OBJECT:
            break;
        case NATIVE_POINTER_OBJECT:
            if (obj->u.native_pointer.info->finalizer) {
                obj->u.native_pointer.info->finalizer(inter, obj);
            }
            break;
        case OBJECT_TYPE_COUNT_PLUS_1:
        default:
            DBG_assert(0, ("bad type..%d\n", obj->type));
    }
    inter->heap.current_heap_size -= sizeof(CRB_Object);
    MEM_free(obj);
}

static void gc_sweep_objects(CRB_Interpreter* inter) {
    for (CRB_Object* obj = inter->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                inter->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            CRB_Object* tmp = obj->next;
            gc_dispose_object(inter, obj);
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
}

void crb_garbage_collect(CRB_Interpreter* inter) {
    gc_mark_objects(inter);
    gc_sweep_objects(inter);
}
