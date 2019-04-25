//
// Created by t5w0rd on 19-4-20.
//

#include <string.h>
#include "tinterpreter.h"
#include "tnative.h"
#include "tmisc.h"
#include "tlexer.h"
#include "theap.h"
#include "texecute.h"
#include "terror.h"
#include "teval.h"


static CRB_Interpreter *st_current_interpreter;

CRB_Interpreter* crb_get_current_interpreter(void) {
    return st_current_interpreter;
}

void crb_set_current_interpreter(CRB_Interpreter *inter) {
    st_current_interpreter = inter;
}

static int asc_order_named_item(VALUE a, VALUE b) {
    return strcmp(((NamedItemEntry*)a.ptr_value)->name, ((NamedItemEntry*)b.ptr_value)->name);
}

CRB_Interpreter* CRB_create_interpreter(void) {
    MEM_Storage storage = MEM_open_storage(0);
    CRB_Interpreter* interpreter = MEM_storage_malloc(storage, sizeof(struct CRB_Interpreter_tag));
    interpreter->interpreter_storage = storage;
    interpreter->execute_storage = MEM_open_storage(0);
    interpreter->global_vars = open_rbtree(asc_order_named_item);
    interpreter->functions = open_rbtree(asc_order_named_item);
    interpreter->statement_list = NULL;
    interpreter->last_statement_pos = NULL;
    interpreter->current_line_number = 1;
    interpreter->stack.stack_alloc_size = 0;
    interpreter->stack.stack_pointer = 0;
    interpreter->stack.stack = MEM_malloc(sizeof(CRB_Value) * STACK_ALLOC_SIZE);
    interpreter->heap.current_heap_size = 0;
    interpreter->heap.current_threshold = HEAP_THRESHOLD_SIZE;
    interpreter->heap.header = NULL;
    interpreter->top_environment = NULL;
    interpreter->current_exception.type = CRB_NULL_VALUE;
    interpreter->input_mode = CRB_FILE_INPUT_MODE;
//    interpreter->regexp_literals = NULL;

#ifdef EUC_SOURCE
    interpreter->source_encoding = EUC_ENCODING;
#else
#ifdef UTF_8_SOURCE
    interpreter->source_encoding = UTF_8_ENCODING;
#else
    DBG_panic(("source encoding is not defined.\n"));
#endif
#endif

    crb_set_current_interpreter(interpreter);
    crb_add_native_functions(interpreter);
//    crb_add_regexp_functions(interpreter);

#ifndef MINICROWBAR
    extern void crb_compile_built_in_script(CRB_Interpreter *inter);
    crb_compile_built_in_script(interpreter);
#endif /* MINICROWBAR */

    return interpreter;
}

static void show_error_stack_trace(CRB_Interpreter *inter) {
    CRB_Object *exception;
    CRB_Value *func;

    if ((setjmp(inter->current_recovery_environment.environment)) == 0) {
        if (inter->current_exception.type != CRB_ASSOC_VALUE) {
            crb_runtime_error(inter, NULL, 0, EXCEPTION_IS_NOT_ASSOC_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        exception = inter->current_exception.u.object;
        func = CRB_search_assoc_member(exception, EXCEPTION_MEMBER_PRINT_STACK_TRACE, NULL);
        if (func == NULL) {
            crb_runtime_error(inter, NULL, 0, EXCEPTION_HAS_NO_PRINT_STACK_TRACE_METHOD_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        if (func->type != CRB_CLOSURE_VALUE) {
            crb_runtime_error(inter, NULL, 0, PRINT_STACK_TRACE_IS_NOT_CLOSURE_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_call_function(inter, NULL, 0, func, 0, NULL);
    } else {
        fprintf(stderr, "Exception occured in print_stack_trace.\n");
        show_error_stack_trace(inter);
    }
}

static void do_compile(CRB_Interpreter *inter) {
    extern int yyparse(void);
    if ((setjmp(inter->current_recovery_environment.environment)) == 0) {
        if (yyparse()) {
            fprintf(stderr, "Error ! Error ! Error !\n");
            exit(1);
        }
    } else {
        show_error_stack_trace(inter);

        crb_set_stack_pointer(inter, 0);
        inter->current_exception.type = CRB_NULL_VALUE;
    }
    crb_garbage_collect(inter);
}

void CRB_compile(CRB_Interpreter *interpreter, FILE *fp) {
    crb_set_current_interpreter(interpreter);
    interpreter->current_line_number = 1;
    interpreter->input_mode = CRB_FILE_INPUT_MODE;

    extern FILE* yyin;
    yyin = fp;

    do_compile(interpreter);

    crb_reset_string_literal_buffer();
}

void CRB_compile_string(CRB_Interpreter *interpreter, const char** lines) {
    crb_set_current_interpreter(interpreter);
    crb_set_source_string(lines);
    interpreter->current_line_number = 1;
    interpreter->input_mode = CRB_STRING_INPUT_MODE;

    do_compile(interpreter);

    crb_reset_string_literal_buffer();
}

void CRB_compile_readline(CRB_Interpreter* interpreter, ReadLineModeParams* params) {
    crb_set_current_interpreter(interpreter);
    crb_set_readline(params);
    interpreter->current_line_number = 1;
    interpreter->input_mode = CRB_READLINE_INPUT_MODE;

    do_compile(interpreter);

    crb_reset_string_literal_buffer();
}

void CRB_set_command_line_args(CRB_Interpreter *interpreter, int argc, char **argv) {
    CRB_Value args;
    args.type = CRB_ARRAY_VALUE;
    args.u.object = crb_create_array_i(interpreter, argc);
    CRB_push_value(interpreter, &args);

    CRB_Value elem;
    for (int i=0; i<argc; ++i) {
        CRB_Char* wc_str = CRB_mbstowcs_alloc(interpreter, NULL, 0, argv[i]);
        if (wc_str == NULL) {
            fprintf(stderr, "bad command line argument(%dth)", i);
            return;
        }
        elem.type = CRB_STRING_VALUE;
        elem.u.object = crb_create_crowbar_string_i(interpreter, wc_str);
        CRB_array_set(interpreter, NULL, args.u.object, i, &elem);
    }
    CRB_add_global_variable(interpreter, "ARGS", &args, CRB_TRUE);

    CRB_pop_value(interpreter);
}

void CRB_interpret(CRB_Interpreter *interpreter) {
    if (interpreter->statement_list == NULL) {
        return;
    }

    crb_add_std_fp(interpreter);
    if ((setjmp(interpreter->current_recovery_environment.environment)) == 0) {
        StatementList* list = interpreter->statement_list;
        struct lnode* last_pos = interpreter->last_statement_pos ? interpreter->last_statement_pos : llist_before_front_node(list);
        interpreter->last_statement_pos = llist_back_node(list);
        StatementResult result = crb_execute_statement_list_with_pos(interpreter, NULL, last_pos);
        if (result.type != NORMAL_STATEMENT_RESULT) {
            crb_runtime_error(interpreter, NULL, 0, BREAK_OR_CONTINUE_REACHED_TOPLEVEL_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
    } else {
        show_error_stack_trace(interpreter);

        crb_set_stack_pointer(interpreter, 0);
        interpreter->current_exception.type = CRB_NULL_VALUE;
    }
    DBG_assert(interpreter->stack.stack_pointer == 0, ("stack_pointer..%d\n", interpreter->stack.stack_pointer));
    crb_garbage_collect(interpreter);
}

static void release_global_strings(CRB_Interpreter *interpreter) {
    rbtree_clear(interpreter->global_vars);
//    while (interpreter->variable) {
//        Variable *temp = interpreter->variable;
//        interpreter->variable = temp->next;
//    }
}

void CRB_dispose_interpreter(CRB_Interpreter *interpreter) {
    release_global_strings(interpreter);

    if (interpreter->execute_storage) {
        MEM_dispose_storage(interpreter->execute_storage);
    }
    close_rbtree(interpreter->global_vars);
    interpreter->global_vars = NULL;
    crb_garbage_collect(interpreter);
    DBG_assert(interpreter->heap.current_heap_size == 0, ("%d bytes leaked.\n", interpreter->heap.current_heap_size));
    MEM_free(interpreter->stack.stack);
//    crb_dispose_regexp_literals(interpreter);
    close_rbtree(interpreter->functions);
    interpreter->functions = NULL;
    MEM_dispose_storage(interpreter->interpreter_storage);
}

CRB_FunctionDefinition* CRB_add_native_function(CRB_Interpreter *interpreter, const char *name, CRB_NativeFunctionProc *proc) {
    CRB_FunctionDefinition* fd = crb_malloc(sizeof(CRB_FunctionDefinition));
    fd->name = name;
    fd->type = CRB_NATIVE_FUNCTION_DEFINE;
    fd->is_closure = CRB_FALSE;
    fd->u.native_f.proc = proc;
    rbtree_set(interpreter->functions, ptr_value(fd));
//    fd->next = interpreter->functions;
//    interpreter->function_list = fd;

    return fd;
}
