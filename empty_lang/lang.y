%{
#include <stdio.h>
#include "lang.h"
#define YYDEBUG 1

extern int yylex();

%}
%union {
    LangValue		value;
}
%token	<value>	IDENTIFIER
%token	<value>	INT_LITERAL
%token	<value>	FLOAT_LITERAL
%token	LP RP SEMICOLON ASSIGN_T ADD SUB MUL DIV
%type	<value>	value

%%
exec_unit
	: statement
	| exec_unit statement
	;

statement
	: value SEMICOLON
	{
		printf("LangValueType: %d\n", $1.type);
	}
	;

value
	: IDENTIFIER
	| INT_LITERAL
	| FLOAT_LITERAL
	;
%%
