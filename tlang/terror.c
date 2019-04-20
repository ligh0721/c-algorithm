//
// Created by t5w0rd on 19-4-20.
//

#include <stdarg.h>
#include <string.h>
#include "tmisc.h"
#include "terror.h"


//static void format_message(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, CRB_ErrorDefinition *format, VString *v, va_list ap) {
//    char        buf[LINE_BUF_SIZE];
//    CRB_Char    wc_buf[LINE_BUF_SIZE];
//    int         arg_name_index;
//    char        arg_name[LINE_BUF_SIZE];
//    MessageArgument     arg[MESSAGE_ARGUMENT_MAX];
//    MessageArgument     cur_arg;
//    CRB_Char    *wc_format;
//
//    create_message_argument(arg, ap);
//
//    wc_format = CRB_mbstowcs_alloc(inter, env, line_number, format->format);
//    DBG_assert(wc_format != NULL, ("wc_format is null.\n"));
//
//    for (int i = 0; wc_format[i] != L'\0'; i++) {
//        if (wc_format[i] != L'$') {
//            crb_vstr_append_character(v, wc_format[i]);
//            continue;
//        }
//        assert(wc_format[i+1] == L'(');
//        i += 2;
//        for (arg_name_index = 0; wc_format[i] != L')';
//             arg_name_index++, i++) {
//            arg_name[arg_name_index] = CRB_wctochar(wc_format[i]);
//        }
//        arg_name[arg_name_index] = '\0';
//        assert(wc_format[i] == L')');
//
//        search_argument(arg, arg_name, &cur_arg);
//        switch (cur_arg.type) {
//            case CRB_INT_MESSAGE_ARGUMENT:
//                sprintf(buf, "%d", cur_arg.u.int_val);
//                CRB_mbstowcs(buf, wc_buf);
//                crb_vstr_append_string(v, wc_buf);
//                break;
//            case CRB_DOUBLE_MESSAGE_ARGUMENT:
//                sprintf(buf, "%f", cur_arg.u.double_val);
//                CRB_mbstowcs(buf, wc_buf);
//                crb_vstr_append_string(v, wc_buf);
//                break;
//            case CRB_STRING_MESSAGE_ARGUMENT:
//                CRB_mbstowcs(cur_arg.u.string_val, wc_buf);
//                crb_vstr_append_string(v, wc_buf);
//                break;
//            case CRB_POINTER_MESSAGE_ARGUMENT:
//                sprintf(buf, "%p", cur_arg.u.pointer_val);
//                CRB_mbstowcs(buf, wc_buf);
//                crb_vstr_append_string(v, wc_buf);
//                break;
//            case CRB_CHARACTER_MESSAGE_ARGUMENT:
//                sprintf(buf, "%c", cur_arg.u.character_val);
//                CRB_mbstowcs(buf, wc_buf);
//                crb_vstr_append_string(v, wc_buf);
//                break;
//            case CRB_MESSAGE_ARGUMENT_END:
//                assert(0);
//                break;
//            default:
//                assert(0);
//        }
//    }
//    MEM_free(wc_format);
//}
//
//static void self_check(void) {
//    if (strcmp(crb_compile_error_message_format[0].format, "dummy") != 0) {
//        DBG_panic(("compile error message format error.\n"));
//    }
//    if (strcmp(crb_compile_error_message_format
//               [COMPILE_ERROR_COUNT_PLUS_1].format,
//               "dummy") != 0) {
//        DBG_panic(("compile error message format error. "
//                   "COMPILE_ERROR_COUNT_PLUS_1..%d\n",
//                COMPILE_ERROR_COUNT_PLUS_1));
//    }
//    if (strcmp(crb_runtime_error_message_format[0].format, "dummy") != 0) {
//        DBG_panic(("runtime error message format error.\n"));
//    }
//    if (strcmp(crb_runtime_error_message_format
//               [RUNTIME_ERROR_COUNT_PLUS_1].format,
//               "dummy") != 0) {
//        DBG_panic(("runtime error message format error. "
//                   "RUNTIME_ERROR_COUNT_PLUS_1..%d\n",
//                RUNTIME_ERROR_COUNT_PLUS_1));
//    }
//}

void crb_runtime_error(CRB_Interpreter *inter, CRB_LocalEnvironment *env, int line_number, RuntimeError id, ...) {
    // TODO:
//    va_list     ap;
//    VString     message;
//
//    self_check();
//    va_start(ap, id);
//    crb_vstr_clear(&message);
//    format_message(inter, env, line_number,
//                   &crb_runtime_error_message_format[id],
//                   &message, ap);
//    va_end(ap);
//
//    throw_runtime_exception(inter, env, line_number, message.string,
//                            &crb_runtime_error_message_format[id]);
}
