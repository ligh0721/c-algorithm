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
%token	SEMICOLON LF LP RP ASSIGN_T ADD SUB MUL DIV
%type	<value>	value

%%
exec_unit
	: statement
	| exec_unit statement
	;

statement
	: expression statement_end
	{
		printf("EXPRESSION_STATEMENT\n");
	}
	| statement_end
	{
		printf("<EMPTY_STATEMENT>\n");
	}
	;

statement_end
	: SEMICOLON
	| LF
	| statement_end SEMICOLON
	| statement_end LF
	;

expression
	: value
	{
		printf("type(%d) ", $1.type);
	}
	;

value
	: IDENTIFIER
	| INT_LITERAL
	| FLOAT_LITERAL
	;
%%
