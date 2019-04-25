//
// Created by t5w0rd on 19-4-20.
//

#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "tinterpreter.h"
#include "tmisc.h"
#include "terror.h"
#include "theap.h"


typedef struct {
    CRB_MessageArgumentType type;
    const char *name;
    union {
        int     int_val;
        double  double_val;
        char    *string_val;
        void    *pointer_val;
        int     character_val;
    } u;
} MessageArgument;

static void create_message_argument(MessageArgument *arg, va_list ap) {
    int index = 0;
    CRB_MessageArgumentType type;

    while ((type = va_arg(ap, CRB_MessageArgumentType)) != CRB_MESSAGE_ARGUMENT_END) {
        arg[index].type = type;
        arg[index].name = va_arg(ap, const char*);
        switch (type) {
            case CRB_INT_MESSAGE_ARGUMENT:
                arg[index].u.int_val = va_arg(ap, int);
                break;
            case CRB_DOUBLE_MESSAGE_ARGUMENT:
                arg[index].u.double_val = va_arg(ap, double);
                break;
            case CRB_STRING_MESSAGE_ARGUMENT:
                arg[index].u.string_val = va_arg(ap, char*);
                break;
            case CRB_POINTER_MESSAGE_ARGUMENT:
                arg[index].u.pointer_val = va_arg(ap, void*);
                break;
            case CRB_CHARACTER_MESSAGE_ARGUMENT:
                arg[index].u.character_val = va_arg(ap, int);
                break;
            case CRB_MESSAGE_ARGUMENT_END:
                assert(0);
                break;
            default:
                assert(0);
        }
        ++index;
        assert(index < MESSAGE_ARGUMENT_MAX);
    }
}

static void search_argument(MessageArgument *arg_list, const char *arg_name, MessageArgument *arg) {
    for (int i=0; arg_list[i].type!=CRB_MESSAGE_ARGUMENT_END; ++i) {
        if (!strcmp(arg_list[i].name, arg_name)) {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void format_message(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_ErrorDefinition *format, VString *v, va_list ap) {
    char buf[LINE_BUF_SIZE];
    CRB_Char wc_buf[LINE_BUF_SIZE];
    char arg_name[LINE_BUF_SIZE];
    MessageArgument arg[MESSAGE_ARGUMENT_MAX];
    create_message_argument(arg, ap);

    CRB_Char* wc_format = CRB_mbstowcs_alloc(inter, env, line_number, format->format);
    DBG_assert(wc_format != NULL, ("wc_format is null. %s(%d)\n", strerror(errno), errno));

    for (int i=0; wc_format[i]!=L'\0'; ++i) {
        if (wc_format[i] != L'$') {
            crb_vstr_append_character(v, wc_format[i]);
            continue;
        }
        assert(wc_format[i+1] == L'(');
        i += 2;
        int arg_name_index;
        for (arg_name_index=0; wc_format[i] != L')'; ++arg_name_index, ++i) {
            arg_name[arg_name_index] = CRB_wctochar(wc_format[i]);
        }
        arg_name[arg_name_index] = '\0';
        assert(wc_format[i] == L')');

        MessageArgument cur_arg;
        search_argument(arg, arg_name, &cur_arg);
        switch (cur_arg.type) {
            case CRB_INT_MESSAGE_ARGUMENT:
                sprintf(buf, "%d", cur_arg.u.int_val);
                CRB_mbstowcs(buf, wc_buf);
                crb_vstr_append_string(v, wc_buf);
                break;
            case CRB_DOUBLE_MESSAGE_ARGUMENT:
                sprintf(buf, "%f", cur_arg.u.double_val);
                CRB_mbstowcs(buf, wc_buf);
                crb_vstr_append_string(v, wc_buf);
                break;
            case CRB_STRING_MESSAGE_ARGUMENT:
                CRB_mbstowcs(cur_arg.u.string_val, wc_buf);
                crb_vstr_append_string(v, wc_buf);
                break;
            case CRB_POINTER_MESSAGE_ARGUMENT:
                sprintf(buf, "%p", cur_arg.u.pointer_val);
                CRB_mbstowcs(buf, wc_buf);
                crb_vstr_append_string(v, wc_buf);
                break;
            case CRB_CHARACTER_MESSAGE_ARGUMENT:
                sprintf(buf, "%c", cur_arg.u.character_val);
                CRB_mbstowcs(buf, wc_buf);
                crb_vstr_append_string(v, wc_buf);
                break;
            case CRB_MESSAGE_ARGUMENT_END:
                assert(0);
                break;
            default:
                assert(0);
        }
    }
    MEM_free(wc_format);
}

extern CRB_ErrorDefinition crb_compile_error_message_format[];
extern CRB_ErrorDefinition crb_runtime_error_message_format[];

static void self_check(void) {
    if (strcmp(crb_compile_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("compile error message format error.\n"));
    }
    if (strcmp(crb_compile_error_message_format[COMPILE_ERROR_COUNT_PLUS_1].format, "dummy") != 0) {
        DBG_panic(("compile error message format error. COMPILE_ERROR_COUNT_PLUS_1..%d\n", COMPILE_ERROR_COUNT_PLUS_1));
    }
    if (strcmp(crb_runtime_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("runtime error message format error.\n"));
    }
    if (strcmp(crb_runtime_error_message_format[RUNTIME_ERROR_COUNT_PLUS_1].format, "dummy") != 0) {
        DBG_panic(("runtime error message format error. RUNTIME_ERROR_COUNT_PLUS_1..%d\n", RUNTIME_ERROR_COUNT_PLUS_1));
    }
}

void crb_compile_error(CompileError id, ...) {
    va_list ap;
    VString message;

    self_check();
    va_start(ap, id);
    CRB_Interpreter* inter = crb_get_current_interpreter();
    int line_number = inter->current_line_number;
    crb_vstr_clear(&message);
    format_message(inter, NULL, line_number, &crb_compile_error_message_format[id], &message, ap);
    fprintf(stderr, "line %d: ", line_number);
    CRB_print_wcs_ln(stderr, message.string);
    va_end(ap);

    if (inter->input_mode != CRB_READLINE_INPUT_MODE) {
        exit(1);
    }
}

static void throw_runtime_exception(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_Char *message, CRB_ErrorDefinition *def) {
    int stack_count = 0;

    CRB_Value* exception_class = CRB_search_global_variable(inter, def->class_name, NULL);

    CRB_Value message_value;
    message_value.type = CRB_STRING_VALUE;
    message_value.u.object = crb_create_crowbar_string_i(inter, message);
    CRB_push_value(inter, &message_value);
    ++stack_count;

    CRB_Value exception_value;
    if (exception_class == NULL) {
        /* for minicrowbar */
        CRB_Object* exception = CRB_create_exception(inter, env, message_value.u.object, line_number);
        exception_value.type = CRB_ASSOC_VALUE;
        exception_value.u.object = exception;
    } else {
        if (exception_class->type != CRB_ASSOC_VALUE) {
            crb_runtime_error(inter, env, line_number, EXCEPTION_CLASS_IS_NOT_ASSOC_ERR, CRB_STRING_MESSAGE_ARGUMENT, "type", CRB_get_type_name(exception_class->type), CRB_MESSAGE_ARGUMENT_END);
        }
        CRB_Value* create_func = CRB_search_assoc_member(exception_class->u.object, EXCEPTION_CREATE_METHOD_NAME, NULL);
        if (create_func == NULL) {
            crb_runtime_error(inter, env, line_number, EXCEPTION_CLASS_HAS_NO_CREATE_METHOD_ERR, CRB_MESSAGE_ARGUMENT_END);
        }
        exception_value = CRB_call_function(inter, env, line_number, create_func, 1, &message_value);
    }
    inter->current_exception = exception_value;

    CRB_shrink_stack(inter, stack_count);

    longjmp(inter->current_recovery_environment.environment, LONGJMP_ARG);
}

void crb_runtime_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, RuntimeError id, ...) {
    va_list ap;
    VString message;

    self_check();
    va_start(ap, id);
    crb_vstr_clear(&message);
    format_message(inter, env, line_number, &crb_runtime_error_message_format[id], &message, ap);
    va_end(ap);

    throw_runtime_exception(inter, env, line_number, message.string, &crb_runtime_error_message_format[id]);
}

void CRB_check_argument_count_func(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, int arg_count, int expected_count) {
    if (arg_count < expected_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_FEW_ERR, CRB_MESSAGE_ARGUMENT_END);
    } else if (arg_count > expected_count) {
        crb_runtime_error(inter, env, line_number, ARGUMENT_TOO_MANY_ERR, CRB_MESSAGE_ARGUMENT_END);
    }
}

void CRB_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_NativeLibInfo *info, int line_number, int error_code, ...) {
    va_list ap;
    VString message;

    va_start(ap, error_code);
    crb_vstr_clear(&message);
    format_message(inter, env, line_number, &info->message_format[error_code], &message, ap);
    throw_runtime_exception(inter, env, line_number, message.string, &info->message_format[error_code]);
}
