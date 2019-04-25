//
// Created by t5w0rd on 2019-04-25.
//

#ifndef ALGORITHM_LANG_H
#define ALGORITHM_LANG_H


int yyerror(char const* str);

typedef enum LangValueType {
    VT_IDENTIFIER,
    VT_INTEGER,
    VT_FLOAT,
} LangValueType;

typedef struct LangValue {
    LangValueType type;
    union {
        const char* identifier;
        long int_value;
        double float_value;
    };
} LangValue;

#endif //ALGORITHM_LANG_H
