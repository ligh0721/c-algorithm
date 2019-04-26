%{
#include <stdio.h>
#include "tinterpreter.h"
#include "terror.h"
#include "tcode.h"
#include "tlexer.h"
#ifdef DEBUG
#define YYDEBUG 1
#endif

extern int yylex();
extern int yyerror(char const *str);

%}
%union {
    const char          *identifier;
    CRB_ParameterList   *parameter_list;
    ArgumentList        *argument_list;
    Expression          *expression;
    ExpressionList      *expression_list;
    AssocExpression     *assoc_expression;
    AssocExpressionList *assoc_expression_list;
    Statement           *statement;
    StatementList       *statement_list;
    CRB_Block           *block;
    Elif                *elif;
    ElifList            *elif_list;
    AssignmentOperator  assignment_operator;
    IdentifierList      *identifier_list;
}
%token  <expression>     INT_LITERAL
%token  <expression>     DOUBLE_LITERAL
%token  <expression>     STRING_LITERAL
%token  <identifier>     IDENTIFIER
%token  FUNCTION IF ELSE ELIF WHILE FOR FOREACH RETURN_T BREAK CONTINUE NULL_T
        LP RP LC RC LB RB LF SEMICOLON COLON COMMA ASSIGN_T LOGICAL_AND LOGICAL_OR
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD TRUE_T FALSE_T EXCLAMATION DOT
        ADD_ASSIGN_T SUB_ASSIGN_T MUL_ASSIGN_T DIV_ASSIGN_T MOD_ASSIGN_T
        INCREMENT DECREMENT CLOSURE GLOBAL_T TRY CATCH FINALLY THROW FINAL
%type   <parameter_list> parameter_list
%type   <argument_list> argument_list
%type   <expression> expression expression_opt
        assignment_expression logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression postfix_expression primary_expression array_literal assoc_literal
        closure_definition
%type   <expression_list> expression_list
%type   <assoc_expression> assoc_expression
%type   <assoc_expression_list> assoc_expression_list
%type   <statement> statement global_statement
        if_statement while_statement for_statement foreach_statement
        return_statement break_statement continue_statement try_statement
        throw_statement
%type   <statement_list> statement_list
%type   <block> block
%type   <elif> elif
%type   <elif_list> elif_list
%type   <assignment_operator> assignment_operator
%type   <identifier> identifier_opt label_opt
%type   <identifier_list> identifier_list
%%
translation_unit
        : definition_or_statement
        | translation_unit definition_or_statement
        | error {
            yyclearin;
            yyerrok;
        }
        ;

definition_or_statement
        : function_definition
        | statement
        {
            CRB_Interpreter* inter = crb_get_current_interpreter();
            inter->statement_list = crb_chain_statement_list(inter->statement_list, $1);
        }
        ;

function_definition
        : FUNCTION IDENTIFIER LP parameter_list RP block
        {
            crb_function_define($2, $4, $6);
        }
        ;

closure_definition
        : CLOSURE IDENTIFIER LP parameter_list RP block
        {
            $$ = crb_create_closure_definition($2, $4, $6);
        }
        | CLOSURE LP parameter_list RP block
        {
            $$ = crb_create_closure_definition(NULL, $3, $5);
        }
        ;

parameter_list
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER
        {
            $$ = crb_create_parameter_list($1);
        }
        | parameter_list COMMA IDENTIFIER
        {
            $$ = crb_chain_parameter_list($1, $3);
        }
        ;

statement_list
        : statement
        {
            $$ = crb_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = crb_chain_statement_list($1, $2);
        }
        ;

expression
        : assignment_expression
        | expression COMMA assignment_expression
        {
            $$ = crb_create_comma_expression($1, $3);
        }
        ;

assignment_expression
        : logical_or_expression
        | postfix_expression assignment_operator assignment_expression
        {
            $$ = crb_create_assign_expression(CRB_FALSE, $1, $2, $3);
        }
        | FINAL postfix_expression assignment_operator assignment_expression
        {
            $$ = crb_create_assign_expression(CRB_TRUE, $2, $3, $4);
        }
        ;

assignment_operator
        : ASSIGN_T
        {
            $$ = NORMAL_ASSIGN;
        }
        | ADD_ASSIGN_T
        {
            $$ = ADD_ASSIGN;
        }
        | SUB_ASSIGN_T
        {
            $$ = SUB_ASSIGN;
        }
        | MUL_ASSIGN_T
        {
            $$ = MUL_ASSIGN;
        }
        | DIV_ASSIGN_T
        {
            $$ = DIV_ASSIGN;
        }
        | MOD_ASSIGN_T
        {
            $$ = MOD_ASSIGN;
        }
        ;

logical_or_expression
        : logical_and_expression
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = crb_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;

logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = crb_create_binary_expression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;

equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = crb_create_binary_expression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = crb_create_binary_expression(NE_EXPRESSION, $1, $3);
        }
        ;

relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = crb_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = crb_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = crb_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = crb_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;

additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = crb_create_binary_expression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = crb_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;

multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = crb_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = crb_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = crb_create_binary_expression(MOD_EXPRESSION, $1, $3);
        }
        ;

unary_expression
        : postfix_expression
        | SUB unary_expression
        {
            $$ = crb_create_minus_expression($2);
        }
        | EXCLAMATION unary_expression
        {
            $$ = crb_create_logical_not_expression($2);
        }
        ;

postfix_expression
        : primary_expression
        | postfix_expression LB expression RB
        {
            $$ = crb_create_index_expression($1, $3);
        }
        | postfix_expression DOT IDENTIFIER
        {
            $$ = crb_create_member_expression($1, $3);
        }
        | postfix_expression LP argument_list RP
        {
            $$ = crb_create_function_call_expression($1, $3);
        }
        | postfix_expression INCREMENT
        {
            $$ = crb_create_inc_dec_expression($1, INCREMENT_EXPRESSION);
        }
        | postfix_expression DECREMENT
        {
            $$ = crb_create_inc_dec_expression($1, DECREMENT_EXPRESSION);
        }
        ;

primary_expression
        : LP expression RP
        {
            $$ = $2;
        }
        | IDENTIFIER
        {
            $$ = crb_create_identifier_expression($1);
        }
        | INT_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | TRUE_T
        {
            $$ = crb_create_boolean_expression(CRB_TRUE);
        }
        | FALSE_T
        {
            $$ = crb_create_boolean_expression(CRB_FALSE);
        }
        | NULL_T
        {
            $$ = crb_create_null_expression();
        }
        | array_literal
        | assoc_literal
        | closure_definition
        ;

argument_list
        : /* empty */
        {
            $$ = NULL;
        }
        | assignment_expression
        {
            $$ = crb_create_argument_list($1);
        }
        | argument_list COMMA assignment_expression
        {
            $$ = crb_chain_argument_list($1, $3);
        }
        ;

array_literal
        : LB expression_list RB
        {
            $$ = crb_create_array_expression($2);
        }
        | LB expression_list COMMA RB
        {
            $$ = crb_create_array_expression($2);
        }
        ;

expression_list
        : /* empty */
        {
            $$ = NULL;
        }
        | assignment_expression
        {
            $$ = crb_create_expression_list($1);
        }
        | expression_list COMMA assignment_expression
        {
            $$ = crb_chain_expression_list($1, $3);
        }
        ;

assoc_literal
        : LC assoc_expression_list RC
        {
            $$ = crb_create_assoc_literal_expression($2);
        }
        | LC assoc_expression_list COMMA RC
        {
            $$ = crb_create_assoc_literal_expression($2);
        }
        ;

assoc_expression_list
        : /* empty */
        {
            $$ = NULL;
        }
        | assoc_expression
        {
            $$ = crb_create_assoc_expression_list($1);
        }
        | assoc_expression_list COMMA assoc_expression
        {
            $$ = crb_chain_assoc_expression_list($1, $3);
        }
        ;

assoc_expression
        : IDENTIFIER COLON assignment_expression
        {
            $$ = crb_create_assoc_expression(CRB_FALSE, $1, $3);
        }
        | FINAL IDENTIFIER COLON assignment_expression
        {
            $$ = crb_create_assoc_expression(CRB_TRUE, $2, $4);
        }
        ;

statement
        : expression statement_end
        {
            $$ = crb_create_expression_statement($1);
        }
        | statement_end
        {
            $$ = crb_create_expression_statement(NULL);
        }
        | global_statement
        | if_statement
        | while_statement
        | for_statement
        | foreach_statement
        | return_statement
        | break_statement
        | continue_statement
        | try_statement
        | throw_statement
        ;

global_statement
        : GLOBAL_T identifier_list statement_end
        {
            $$ = crb_create_global_statement($2);
        }
        ;

identifier_list
        : IDENTIFIER
        {
            $$ = crb_create_global_identifier_list($1);
        }
        | identifier_list COMMA IDENTIFIER
        {
            $$ = crb_chain_global_identifier_list($1, $3);
        }
        ;

return_statement
        : RETURN_T expression_opt statement_end
        {
            $$ = crb_create_return_statement($2);
        }
        ;

if_statement
        : IF LP expression RP block
        {
            $$ = crb_create_if_statement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = crb_create_if_statement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elif_list
        {
            $$ = crb_create_if_statement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elif_list ELSE block
        {
            $$ = crb_create_if_statement($3, $5, $6, $8);
        }
        ;

elif_list
        : elif
        {
            $$ = crb_create_elif_list($1);
        }
        | elif_list elif
        {
            $$ = crb_chain_elif_list($1, $2);
        }
        ;

elif
        : ELIF LP expression RP block
        {
            $$ = crb_create_elif($3, $5);
        }
        ;

label_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER COLON
        {
            $$ = $1;
        }
        ;

while_statement
        : label_opt WHILE LP expression RP block
        {
            $$ = crb_create_while_statement($1, $4, $6);
        }
        ;

for_statement
        : label_opt FOR LP expression_opt SEMICOLON expression_opt SEMICOLON expression_opt RP block
        {
            $$ = crb_create_for_statement($1, $4, $6, $8, $10);
        }
        ;

foreach_statement
        : label_opt FOREACH LP IDENTIFIER COLON expression RP block
        {
            $$ = crb_create_foreach_statement($1, $4, $6, $8);
        }
        ;

expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;

identifier_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER
        ;

break_statement
        : BREAK identifier_opt statement_end
        {
            $$ = crb_create_break_statement($2);
        }
        ;

continue_statement
        : CONTINUE identifier_opt statement_end
        {
            $$ = crb_create_continue_statement($2);
        }
        ;

try_statement
        : TRY block CATCH LP IDENTIFIER RP block FINALLY block
        {
            $$ = crb_create_try_statement($2, $5, $7, $9);
        }
        | TRY block FINALLY block
        {
            $$ = crb_create_try_statement($2, NULL, NULL, $4);
        }
        | TRY block CATCH LP IDENTIFIER RP block
        {
            $$ = crb_create_try_statement($2, $5, $7, NULL);
        }
        ;

throw_statement
        : THROW expression statement_end
        {
            $$ = crb_create_throw_statement($2);
        }
        ;

block
        : LC statement_list RC
        {
            $$ = crb_create_block($2);
        }
        | LC RC {
            $$ = crb_create_block(NULL);
        }
        ;
        
statement_end
        : SEMICOLON
        | LF
        ;
%%
