//
// Created by t5w0rd on 2019-04-25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lang.h"



int yywrap(void) {
    return 1;
}

int yyerror(char const* str) {
    perror(str);
    return 0;
}

int main(int argc, char** args) {
    extern int yyparse(void);
    if (yyparse()) {
        fprintf(stderr, "Error ! Error ! Error !\n");
        exit(1);
    }
    return 0;
}
