%{
#include "lang.h"
#include "lang_y.tab.h"


%}
%start STRING_LITERAL_STATE
%%
<INITIAL>"("            return LP;
<INITIAL>")"            return RP;
<INITIAL>";"            return SEMICOLON;
<INITIAL>"="            return ASSIGN_T;
<INITIAL>"+"            return ADD;
<INITIAL>"-"            return SUB;
<INITIAL>"*"            return MUL;
<INITIAL>"/"            return DIV;
<INITIAL>[A-Za-z_][A-Za-z_0-9]* {
    LangValue v;
    v.type = VT_IDENTIFIER;
    v.identifier = strdup(yytext);
    yylval.value = v;
    return IDENTIFIER;
}
<INITIAL>[1-9][0-9]* {
    LangValue v;
    v.type = VT_INTEGER;
    sscanf(yytext, "%ld", &v.int_value);
    yylval.value = v;
    return INT_LITERAL;
}
<INITIAL>"0" {
    LangValue v;
    v.type = VT_INTEGER;
    v.int_value = 0;
    yylval.value = v;
    return INT_LITERAL;
}
<INITIAL>((?:[1-9][0-9]*)|0)\.[0-9]+ {
    LangValue v;
    v.type = VT_FLOAT;
    sscanf(yytext, "%lf", &v.float_value);
    yylval.value = v;
    return FLOAT_LITERAL;
}
<INITIAL>[ \t] ;
<INITIAL>[ \t\r\n] {}
<INITIAL>.      {
    yyerror("lex error\n");
}

%%
