//
// Created by t5w0rd on 19-4-19.
//

#include <stdio.h>
#include <algorithm.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <locale.h>
#include <include/tlang.h>
#include "tlang.h"


static char* my_readline(void* param) {
    return readline((const char*)param);
}

static void my_add_history(const char* history, void* param) {
    add_history(history);
}

int readline_mode(int argc, char* argv[]) {
    const char* ps = ">> ";
    CRB_Interpreter* interpreter = CRB_create_interpreter();
    CRB_set_command_line_args(interpreter, argc-1, &argv[1]);
    ReadLineModeParams params = {my_readline, (void*)ps, my_add_history, NULL};
    CRB_compile_readline(interpreter, &params);
    CRB_dispose_interpreter(interpreter);
    MEM_dump_blocks(stdout);
    return 0;
}

int file_mode(int argc, char* argv[]) {
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        return 1;
    }
    CRB_Interpreter* interpreter = CRB_create_interpreter();
    CRB_set_command_line_args(interpreter, argc-2, &argv[2]);
    CRB_compile(interpreter, fp);
    CRB_interpret(interpreter);
    CRB_dispose_interpreter(interpreter);
    MEM_dump_blocks(stdout);
    fclose(fp);
    return 0;
}

CRB_Value return_value(int i) {
    CRB_Value value;
    value.type = CRB_INT_VALUE;
    value.u.int_value = i*i;
    return value;
}

void return_value2(int i, CRB_Value* value) {
    value->type = CRB_INT_VALUE;
    value->u.int_value = i*i;
}

void test() {
    clock_t t = clock();
    long s = 0;
    CRB_Value value;
    for (int i=0; i<100000000; ++i) {
        return_value2(i, &value);
//        value = return_value(i);
        s += value.u.int_value;
    }
    printf("cose: %ld ticks, %ld\n", clock()-t, s);
    exit(0);
}

int main(int argc, char* argv[]) {
//    test();
    setlocale(LC_CTYPE, "");
    if (argc == 1) {
        return readline_mode(argc, argv);
    } else {
        return file_mode(argc, argv);
    }
}
