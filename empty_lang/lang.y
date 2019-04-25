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
	: statement_list
	| exec_unit statement_list
	;

statement_list
	: statement
	| statement_list statement
	;

statement
	: expression statement_end
	{
		fprintf(stderr, "<EXPRESSION_STATEMENT>\n");
	}
	| statement_end
	{
		fprintf(stderr, "<EMPTY_STATEMENT>\n");
	}
	;

statement_end
	: SEMICOLON
	| LF
	;

expression
	: value
	{
		fprintf(stderr, "type(%d) ", $1.type);
	}
	;

value
	: IDENTIFIER
	| INT_LITERAL
	| FLOAT_LITERAL
	;
%%
