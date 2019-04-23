//
// Created by t5w0rd on 19-4-20.
//

#include <wchar.h>
#include <string.h>
#include "tinterpreter.h"
#include "theap.h"


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

CRB_Object* crb_create_native_pointer_i(CRB_Interpreter *inter, void *pointer, CRB_NativePointerInfo *info) {
    CRB_Object* ret = alloc_object(inter, NATIVE_POINTER_OBJECT);
    ret->u.native_pointer.pointer = pointer;
    ret->u.native_pointer.info = info;
    return ret;
}

// assoc object
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
//    AssocMember* member_p = MEM_realloc(assoc->u.assoc.members, sizeof(AssocMember) * (assoc->u.assoc.member_count+1));
//    member_p[assoc->u.assoc.member_count].name = name;
//    member_p[assoc->u.assoc.member_count].value = *value;
//    member_p[assoc->u.assoc.member_count].is_final = is_final;
//    assoc->u.assoc.member = member_p;
//    assoc->u.assoc.member_count++;
//    inter->heap.current_heap_size += sizeof(AssocMember);
//    return &member_p[assoc->u.assoc.member_count-1].value;
}

CRB_Value* CRB_search_assoc_member(CRB_Object *assoc, const char *member_name, CRB_Boolean* is_final) {
    AssocMember_RBTREE* tr = assoc->u.assoc.members;
    AssocMember key = {member_name};
    AssocMember_RBNODE** where = AssocMember_rbtree_fast_get(tr, key, NULL);
    if (AssocMember_rbtree_node_not_found(tr, where)) {
        return NULL;
    }
    AssocMember* value_entry = AssocMember_rbtree_fast_value(tr, where);
    return &value_entry->value;
//    if (assoc->u.assoc.member_count == 0) {
//        return NULL;
//    }
//
//    for (int i = 0; i < assoc->u.assoc.member_count; i++) {
//        if (!strcmp(assoc->u.assoc.member[i].name, member_name)) {
//            AssocMember* ret = assoc->u.assoc.member+i;
//            if (is_final) {
//                *is_final = ret->is_final;
//            }
//            return &ret->value;
//        }
//    }
//    return NULL;
}

// gc
static void gc_mark_value(CRB_Value *v);

static int _gc_mark_every_assoc_member(const AssocMember *value, void *param) {
    gc_mark_value(&((AssocMember*)value)->value);
    return 0;
}

static void gc_mark(CRB_Object *obj) {
    if (obj == NULL)
        return;

    if (obj->marked)
        return;

    obj->marked = CRB_TRUE;

    if (obj->type == ARRAY_OBJECT) {
        CRB_Value_ARRAY* arr = obj->u.array.array;
        long len = CRB_Value_array_len(arr);
        CRB_Value* data = CRB_Value_array_data(arr);
        for (long i=0; i<len; ++i) {
            gc_mark_value(data++);
        }
//        for (int i = 0; i < obj->u.array.size; i++) {
//            gc_mark_value(&obj->u.array.array[i]);
//        }
    } else if (obj->type == ASSOC_OBJECT) {
        AssocMember_RBTREE* tr = obj->u.assoc.members;
        AssocMember_rbtree_ldr(tr, _gc_mark_every_assoc_member, NULL);
//        for (int i = 0; i < obj->u.assoc.member_count; i++) {
//            gc_mark_value(&obj->u.assoc.member[i].value);
//        }
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
        if (v->u.closure.environment) {
            gc_mark(v->u.closure.environment);
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

static int _every_variable_gc_mark_value(const VALUE* value, void *param) {
    Variable* v = (Variable*)value->ptr_value;
    gc_mark_value(&v->value);
    return 0;
}

static void gc_mark_objects(CRB_Interpreter *inter) {
    for (CRB_Object* obj = inter->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }

    if (inter->variables != NULL) {
        rbtree_ldr(inter->variables, _every_variable_gc_mark_value, NULL);
    }
//    for (Variable* v = inter->variable; v; v = v->next) {
//        gc_mark_value(&v->value);
//    }

    for (CRB_LocalEnvironment* lv = inter->top_environment; lv; lv = lv->next) {
        gc_mark(lv->variable);
        gc_mark_ref_in_native_method(lv);
    }

    for (int i = 0; i < inter->stack.stack_pointer; i++) {
        gc_mark_value(&inter->stack.stack[i]);
    }

    gc_mark_value(&inter->current_exception);
}

static void gc_dispose_object(CRB_Interpreter *inter, CRB_Object *obj) {
    switch (obj->type) {
        case ARRAY_OBJECT:
            inter->heap.current_heap_size -= sizeof(CRB_Value) * CRB_Value_array_cap(obj->u.array.array);
            MEM_free(obj->u.array.array);
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
            obj->u.assoc.members = NULL;
//            MEM_free(obj->u.assoc.member);
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
    CRB_Object *obj;
    CRB_Object *tmp;

    for (obj = inter->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                inter->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
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
