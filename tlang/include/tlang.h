//
// Created by t5w0rd on 19-4-19.
//

#ifndef TLANG_TLANG_H
#define TLANG_TLANG_H

#include <stdio.h>
#include <wchar.h>
#include <link.h>
#include <rbtree.h>

#define UTF_8_SOURCE


// tinterpreter
typedef struct CRB_Interpreter_tag CRB_Interpreter;

typedef char* (*READLINE_FUNC)(void* param);
typedef void (*ADD_HISTORY_FUNC)(const char* history, void* param);
typedef struct {
    READLINE_FUNC readline;
    void* readline_params;
    ADD_HISTORY_FUNC add_history;
    void* add_history_params;
} ReadLineModeParams;

CRB_Interpreter *CRB_create_interpreter(void);
void CRB_compile(CRB_Interpreter* interpreter, FILE *fp);
void CRB_compile_string(CRB_Interpreter* interpreter, const char** lines);
void CRB_compile_builtin_string(CRB_Interpreter *interpreter, const char** lines);
void CRB_compile_readline(CRB_Interpreter* interpreter, ReadLineModeParams* params);
void CRB_set_command_line_args(CRB_Interpreter* interpreter, int argc, char** argv);
void CRB_interpret(CRB_Interpreter* interpreter);
void CRB_dispose_interpreter(CRB_Interpreter* interpreter);
void CRB_import_model(CRB_Interpreter* interpreter, const char* name);


// development
typedef wchar_t CRB_Char;

typedef enum {
    CRB_FALSE = 0,
    CRB_TRUE = 1
} CRB_Boolean;

typedef enum {
    CRB_NULL_VALUE = 1,
    CRB_BOOLEAN_VALUE,
    CRB_INT_VALUE,
    CRB_FLOAT_VALUE,
    CRB_STRING_VALUE,
    CRB_NATIVE_POINTER_VALUE,
    CRB_ARRAY_VALUE,
    CRB_ASSOC_VALUE,
    CRB_CLOSURE_VALUE,
    CRB_FAKE_METHOD_VALUE,
    CRB_SCOPE_CHAIN_VALUE,
    CRB_MODULE_VALUE,
} CRB_ValueType;

typedef struct CRB_Object_tag CRB_Object;
typedef struct CRB_Array_tag CRB_Array;
typedef struct CRB_String_tag CRB_String;
typedef struct CRB_Assoc_tag CRB_Assoc;
typedef LLIST CRB_ParameterList;  // LLIST<char*>
typedef struct CRB_Block_tag CRB_Block;
typedef struct CRB_FunctionDefinition_tag CRB_FunctionDefinition;
typedef struct CRB_LocalEnvironment_tag CRB_LocalEnvironment;
typedef struct CRB_Module_tag CRB_Module;

#define CRB_env_module(inter, env) ((env) ? (env)->module : (inter)->current_module)
#define CRB_global_vars(inter, module) ((module) ? (module)->global_vars : (inter)->global_vars)
#define CRB_global_funcs(inter, module) ((module) ? (module)->global_funcs : (inter)->global_funcs)

typedef struct {
    CRB_FunctionDefinition* function_definition;
    CRB_Object*             scope_chain; /* CRB_ScopeChain */
} CRB_Closure;

typedef struct {
    const char* method_name;
    CRB_Object* object;
} CRB_FakeMethod;

typedef struct {
    CRB_ValueType       type;
    union {
        CRB_Boolean     boolean_value;
        long            int_value;
        double          float_value;
        CRB_Object*     object;
        CRB_Closure     closure;
        CRB_FakeMethod  fake_method;
        CRB_Module*     module;
    } u;
} CRB_Value;

extern CRB_Value CRB_Null_Value;
extern CRB_Value CRB_True_Value;
extern CRB_Value CRB_False_Value;

typedef enum {
    CRB_CROWBAR_FUNCTION_DEFINE = 1,
    CRB_NATIVE_FUNCTION_DEFINE,
    CRB_FUNCTION_DEFINE_TYPE_COUNT_PLUS_1
} CRB_FunctionDefinitionType;

typedef void CRB_NativeFunctionFunc(CRB_Interpreter *interpreter, CRB_LocalEnvironment *env, int arg_count, CRB_Value *args, CRB_Value *result);

struct CRB_FunctionDefinition_tag {
    const char*                 name;
    CRB_FunctionDefinitionType  type;
    CRB_Boolean                 is_closure;
    union {
        struct {
            CRB_ParameterList*  parameter;
            CRB_Block*          block;
        } crowbar_f;
        struct {
            int                         param_count;
            CRB_NativeFunctionFunc*     func;
        } native_f;
    } u;
    CRB_Module*                 module;
    struct CRB_FunctionDefinition_tag*  next;
};

struct CRB_Module_tag {
    const char* name;
    RBTREE*     global_vars;  // RBTREE<Variable*>
    RBTREE*     global_funcs;  // RBTREE<CRB_FunctionDefinition*>
};

// Error
typedef struct {
    const char* format;
    const char* class_name;
} CRB_ErrorDefinition;

typedef struct {
    CRB_ErrorDefinition* message_format;
} CRB_NativeLibInfo;

void CRB_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_NativeLibInfo *info, int line_number, int error_code, ...);

typedef void CRB_NativePointerFinalizeProc(CRB_Interpreter *inter, CRB_Object *obj);
typedef struct {
    const char                          *name;
    CRB_NativePointerFinalizeProc       *finalizer;
} CRB_NativePointerInfo;

typedef enum {
    CRB_INT_MESSAGE_ARGUMENT = 1,
    CRB_DOUBLE_MESSAGE_ARGUMENT,
    CRB_STRING_MESSAGE_ARGUMENT,
    CRB_CHARACTER_MESSAGE_ARGUMENT,
    CRB_POINTER_MESSAGE_ARGUMENT,
    CRB_MESSAGE_ARGUMENT_END
} CRB_MessageArgumentType;

// theap
CRB_Object* CRB_create_crowbar_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Char *str);

CRB_Object* CRB_create_array(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int size);

CRB_Object* CRB_create_assoc(CRB_Interpreter *inter, CRB_LocalEnvironment *env);
CRB_Value* CRB_add_assoc_member(CRB_Interpreter *inter, CRB_Object *assoc, const char *name, CRB_Value *value, CRB_Boolean is_final);
CRB_Value* CRB_search_assoc_member(CRB_Object *assoc, const char *member_name, CRB_Boolean* is_final);

CRB_Object* CRB_create_native_pointer(CRB_Interpreter *inter, CRB_LocalEnvironment *env, void *pointer, const CRB_NativePointerInfo *info);
const CRB_NativePointerInfo* CRB_get_native_pointer_type(CRB_Object *native_pointer);
CRB_Boolean CRB_check_native_pointer_type(CRB_Object *native_pointer, const CRB_NativePointerInfo *info);

CRB_Object* CRB_create_exception(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *message, int line_number);

// teval
void CRB_push_value(CRB_Interpreter *inter, CRB_Value *value);
void CRB_pop_value(CRB_Interpreter *inter, CRB_Value *poped);
CRB_Value* CRB_peek_stack(CRB_Interpreter *inter, int index);
void CRB_shrink_stack(CRB_Interpreter *inter, int shrink_size);
void CRB_create_closure(CRB_LocalEnvironment *env, CRB_FunctionDefinition *fd, CRB_Value *result);
void CRB_call_function(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Value *func, int arg_count, CRB_Value *args, CRB_Value *result);
void CRB_call_function_by_name(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *func_name, int arg_count, CRB_Value *args, CRB_Value *result);
void CRB_call_method(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Object *obj, const char *method_name, int arg_count, CRB_Value *args, CRB_Value *result);

// tmisc
CRB_Value* CRB_add_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier, CRB_Value *value, CRB_Boolean is_final);
CRB_Value* CRB_search_global_variable(CRB_Interpreter *inter, CRB_Module* module, const char *identifier, CRB_Boolean *is_final);
CRB_Value* CRB_add_local_variable(CRB_Interpreter *inter, CRB_LocalEnvironment *env, const char *identifier, CRB_Value *value, CRB_Boolean is_final);
CRB_Value* CRB_search_local_variable(CRB_LocalEnvironment *env, const char *identifier, CRB_Boolean *is_final);
CRB_FunctionDefinition* CRB_search_function(CRB_Interpreter *inter, CRB_Module* module, const char *name);
CRB_Module* CRB_add_module(CRB_Interpreter* inter, const char* name);
CRB_Module* CRB_add_module_if_not_exist(CRB_Interpreter* inter, const char* name, CRB_Boolean* exist);
CRB_Module* CRB_search_module(CRB_Interpreter* inter, const char* name);
void* CRB_object_get_native_pointer(CRB_Object *obj);

// twchar
#define CRB_wcslen(s) wcslen(s)
#define CRB_wcscpy(dst, src) wcscpy(dst, src)
#define CRB_wcsncpy(dst, src, n) wcsncpy(dst, src, n)
#define CRB_wcscat(s, s2) wcscat(s, s2)
#define CRB_wcscmp(s, s2) wcscmp(s, s2)
int CRB_mbstowcs_len(const char *src);
void CRB_mbstowcs(const char *src, CRB_Char *dest);
CRB_Char* CRB_mbstowcs_alloc(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const char *src);
int CRB_wcstombs_len(const CRB_Char *src);
void CRB_wcstombs(const CRB_Char *src, char *dest);
char* CRB_wcstombs_alloc(const CRB_Char *src);
char CRB_wctochar(CRB_Char src);
int CRB_print_wcs(FILE *fp, const CRB_Char *str);
int CRB_print_wcs_ln(FILE *fp, const CRB_Char *str);
CRB_Char* CRB_value_to_string(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, const CRB_Value *value, void* param);

// tnative
CRB_Char* CRB_object_get_string(CRB_Object *obj);
void* CRB_object_get_native_pointer(CRB_Object *obj);
void CRB_array_set(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int index, CRB_Value *value);
void CRB_array_append(CRB_Interpreter *inter, CRB_Object *obj, CRB_Value *new_value);
void CRB_array_insert(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int pos, CRB_Value *new_value, int line_number);
void CRB_array_pop(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int pos, int line_number, CRB_Value *popped);
CRB_FunctionDefinition* CRB_add_native_function(CRB_Interpreter *interpreter, CRB_Module *module, const char *name, int param_count, CRB_NativeFunctionFunc *func);
void CRB_set_native_function(CRB_Module *module, const char *name, int param_count,
                             CRB_NativeFunctionFunc *func, CRB_FunctionDefinition *fd);
void CRB_check_argument_count(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, int arg_count, int expected_count);
//#define CRB_check_argument_count_in_native(inter, env, arg_count, expected_count) (CRB_check_argument_count(inter, env, __LINE__, arg_count, expected_count))



// memory
typedef enum {
    MEM_FAIL_AND_EXIT,
    MEM_FAIL_AND_RETURN
} MEM_FailMode;

typedef struct MEM_Controller_tag *MEM_Controller;
typedef void (*MEM_ErrorHandler)(MEM_Controller, const char *, int, const char *);
typedef struct MEM_Storage_tag *MEM_Storage;

extern MEM_Controller mem_default_controller;

#ifdef MEM_CONTROLLER
#define MEM_CURRENT_CONTROLLER MEM_CONTROLLER
#else /* MEM_CONTROLLER */
#define MEM_CURRENT_CONTROLLER mem_default_controller
#endif /* MEM_CONTROLLER */

/*
 * Don't use mem_*_func function.
 * There are private functions of MEM module.
 */
MEM_Controller MEM_create_controller(void);
void *MEM_malloc_func(MEM_Controller controller, const char *filename, int line, size_t size);
void *MEM_realloc_func(MEM_Controller controller, const char *filename, int line, void *ptr, size_t size);
const char *MEM_strdup_func(MEM_Controller controller, const char *filename, int line, const char *str);
MEM_Storage MEM_open_storage_func(MEM_Controller controller, const char *filename, int line, int page_size);
void *MEM_storage_malloc_func(MEM_Controller controller, const char *filename, int line, MEM_Storage storage, size_t size);
void MEM_free_func(MEM_Controller controller, void *ptr);
void MEM_dispose_storage_func(MEM_Controller controller, MEM_Storage storage);

void MEM_set_error_handler(MEM_Controller controller, MEM_ErrorHandler handler);
void MEM_set_fail_mode(MEM_Controller controller,MEM_FailMode mode);
void MEM_dump_blocks_func(MEM_Controller controller, FILE *fp);
void MEM_check_block_func(MEM_Controller controller, const char *filename, int line, void *p);
void MEM_check_all_blocks_func(MEM_Controller controller, const char *filename, int line);

#define MEM_malloc(size) (MEM_malloc_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, size))
#define MEM_realloc(ptr, size) (MEM_realloc_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, ptr, size))
#define MEM_strdup(str) (MEM_strdup_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, str))
#define MEM_open_storage(page_size) (MEM_open_storage_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, page_size))
#define MEM_storage_malloc(storage, size) (MEM_storage_malloc_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, storage, size))
#define MEM_free(ptr) (MEM_free_func(MEM_CURRENT_CONTROLLER, ptr))
#define MEM_dispose_storage(storage) (MEM_dispose_storage_func(MEM_CURRENT_CONTROLLER, storage))
#ifdef DEBUG
#define MEM_dump_blocks(fp) (MEM_dump_blocks_func(MEM_CURRENT_CONTROLLER, fp))
#define MEM_check_block(p) (MEM_check_block_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__, p))
#define MEM_check_all_blocks() (MEM_check_all_blocks_func(MEM_CURRENT_CONTROLLER, __FILE__, __LINE__))
#else /* DEBUG */
#define MEM_dump_blocks(fp) ((void)0)
#define MEM_check_block(p)  ((void)0)
#define MEM_check_all_blocks() ((void)0)
#endif /* DEBUG */

// debug
typedef struct DBG_Controller_tag *DBG_Controller;
void DBG_set(DBG_Controller controller, const char *file, int line);
void DBG_set_expression(const char *expression);

#ifdef DBG_NO_DEBUG
#define DBG_create_controller()         ((void)0)
#define DBG_set_debug_level(level)      ((void)0)
#define DBG_set_debug_write_fp(fp)      ((void)0)
#define DBG_assert(expression, arg)     ((void)0)
#define DBG_panic(arg)                  ((void)0)
#define DBG_debug_write(arg)            ((void)0)
#else /* DBG_NO_DEBUG */
#ifdef DBG_CONTROLLER
#define DBG_CURRENT_CONTROLLER  DBG_CONTROLLER
#else /* DBG_CONTROLLER */
#define DBG_CURRENT_CONTROLLER  dbg_default_controller
#endif /* DBG_CONTROLLER */
extern DBG_Controller DBG_CURRENT_CONTROLLER;

#define DBG_create_controller() (DBG_create_controller_func())
#define DBG_set_debug_level(level) (DBG_set_debug_level_func(DBG_CURRENT_CONTROLLER, (level)))
#define DBG_set_debug_write_fp(fp) (DBG_set_debug_write_fp(DBG_CURRENT_CONTROLLER, (fp))
#define DBG_assert(expression, arg) ((expression) ? (void)(0) : ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), (DBG_set_expression(#expression)), DBG_assert_func arg))
#define DBG_panic(arg) ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), DBG_panic_func arg)
#define DBG_debug_write(arg) ((DBG_set(DBG_CURRENT_CONTROLLER, __FILE__, __LINE__)), DBG_debug_write_func arg)
#endif /* DBG_NO_DEBUG */

typedef enum {
    DBG_TRUE = 1,
    DBG_FALSE = 0
} DBG_Boolean;

DBG_Controller DBG_create_controller_func(void);
void DBG_set_debug_level_func(DBG_Controller controller, int level);
void DBG_set_debug_write_fp_func(DBG_Controller controller, FILE *fp);
void DBG_assert_func(const char *fmt, ...);
void DBG_panic_func(const char *fmt, ...);
void DBG_debug_write_func(int level, const char *fmt, ...);

#endif //TLANG_TLANG_H
