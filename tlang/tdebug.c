//
// Created by t5w0rd on 19-4-20.
//

#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tdebug.h"


static DBG_Controller st_current_controller;
static const char* st_current_file_name;
static int st_current_line;
static const char* st_assert_expression;

static struct DBG_Controller_tag st_default_controller = {
        NULL, /*stderr,*/
        INT_MAX,
};

DBG_Controller dbg_default_controller = &st_default_controller;

DBG_Controller DBG_create_controller_func(void) {
    DBG_Controller controller = MEM_malloc(sizeof(struct DBG_Controller_tag));
    controller->debug_write_fp = NULL;
    controller->current_debug_level = INT_MAX;
    return controller;
}

void DBG_set_debug_level_func(DBG_Controller controller, int level) {
    controller->current_debug_level = level;
}

void DBG_set_debug_write_fp_func(DBG_Controller controller, FILE *fp) {
    controller->debug_write_fp = fp;
}

static inline void initialize_debug_write_fp(void) {
    if (st_default_controller.debug_write_fp == NULL) {
        st_default_controller.debug_write_fp = stderr;
    }
}

static inline void assert_func(FILE *fp, const char *file, int line, const char *expression, const char *fmt,  va_list ap) {
    fprintf(fp, "Assertion failure (%s)\n%s:%d\n", expression, file, line);
    if (fmt) {
        vfprintf(fp, fmt, ap);
    }
}

void DBG_assert_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    initialize_debug_write_fp();
    assert_func(st_current_controller->debug_write_fp, st_current_file_name, st_current_line, st_assert_expression, fmt, ap);
    assert_func(stderr, st_current_file_name, st_current_line, st_assert_expression, fmt, ap);
    va_end(ap);
    abort();
}

static inline void panic_func(FILE *fp, const char *file, int line, const char *fmt,  va_list ap) {
    fprintf(fp, "Panic!!\n%s:%d\n", file, line);
    if (fmt) {
        vfprintf(fp, fmt, ap);
    }
}

void DBG_panic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    initialize_debug_write_fp();
    panic_func(st_current_controller->debug_write_fp, st_current_file_name, st_current_line, fmt, ap);
    panic_func(stderr, st_current_file_name, st_current_line, fmt, ap);
    va_end(ap);
    abort();
}

void DBG_debug_write_func(int level, const char *fmt, ...) {
    va_list ap;
    if (level > 0 && level > st_current_controller->current_debug_level) {
        return;
    }
    va_start(ap, fmt);
    initialize_debug_write_fp();
    if (fmt) {
        vfprintf(st_current_controller->debug_write_fp, fmt, ap);
    }
    va_end(ap);
}

void DBG_set(DBG_Controller controller, const char *file, int line) {
    st_current_controller = controller;
    st_current_file_name = file;
    st_current_line = line;
}

void DBG_set_expression(const char *expression) {
    st_assert_expression = expression;
}
