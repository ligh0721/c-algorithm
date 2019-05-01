//
// Created by t5w0rd on 19-4-20.
//

#ifndef TLANG_TINTERPRETER_H
#define TLANG_TINTERPRETER_H

#include <link.h>
#include <rbtree.h>
#include <wstring.h>
#include <setjmp.h>
#include "tobject.h"

#define smaller(a, b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE           (1024)
#define STACK_ALLOC_SIZE        (256)
#define HEAP_THRESHOLD_SIZE     (1024 * 256)
#define LONGJMP_ARG             (1)


typedef struct {
    const char* name;
} NamedItemEntry;

typedef struct {
    const char* name;
    CRB_Value value;
    CRB_Boolean is_final;
} Variable;

typedef enum {
    NORMAL_STATEMENT_RESULT = 1,
    RETURN_STATEMENT_RESULT,
    BREAK_STATEMENT_RESULT,
    CONTINUE_STATEMENT_RESULT,
    STATEMENT_RESULT_TYPE_COUNT_PLUS_1
} StatementResultType;

typedef struct {
    StatementResultType type;
    union {
        CRB_Value       return_value;
        const char      *label;
    } u;
} StatementResult;


// object
struct CRB_Array_tag {
    CRB_Value_SLICE* array;  //  SLICE<CRB_Value>
};

struct CRB_String_tag {
    CRB_Boolean is_literal;
//    WSTRING* string;
    CRB_Char    *string;
};

struct AssocMember_tag {
    const char  *name;
    CRB_Value   value;
    CRB_Boolean is_final;
};

struct CRB_Assoc_tag {
    AssocMember_RBTREE* members;  // RBTREE<AssocMember>
};

typedef struct {
    CRB_Object  *frame; /* CRB_Assoc */
    CRB_Object  *next;  /* ScopeChain */
} ScopeChain;

typedef struct {
    void                        *pointer;
    const CRB_NativePointerInfo *info;
} NativePointer;

typedef enum {
    ARRAY_OBJECT = 1,
    STRING_OBJECT,
    ASSOC_OBJECT,
    SCOPE_CHAIN_OBJECT,
    NATIVE_POINTER_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct CRB_Object_tag {
    ObjectType  type;
    unsigned int        marked:1;
    union {
        CRB_Array       array;
        CRB_String      string;
        CRB_Assoc       assoc;
        ScopeChain      scope_chain;
        NativePointer   native_pointer;
    } u;
    struct CRB_Object_tag *prev;
    struct CRB_Object_tag *next;
};

#define crb_is_object_value(type) ((type) == CRB_STRING_VALUE || (type) == CRB_ARRAY_VALUE || (type) == CRB_ASSOC_VALUE || (type) == CRB_NATIVE_POINTER_VALUE || (type) == CRB_SCOPE_CHAIN_VALUE)

// Method
#define ARRAY_ITERATOR_METHOD_NAME  ("__create_array_iterator")
#define ITERATOR_METHOD_NAME        ("iterator")
#define IS_DONE_METHOD_NAME         ("is_done")
#define NEXT_METHOD_NAME            ("next")
#define CURRENT_ITEM_METHOD_NAME    ("current_item")

// Error
#define EXCEPTION_MEMBER_MESSAGE                ("message")
#define EXCEPTION_MEMBER_STACK_TRACE            ("stack_trace")
#define EXCEPTION_MEMBER_LINE_NUMBER            ("line_number")
#define EXCEPTION_MEMBER_FUNCTION_NAME          ("function_name")
#define EXCEPTION_MEMBER_PRINT_STACK_TRACE      ("print_stack_trace")
#define EXCEPTION_CREATE_METHOD_NAME            ("create")
#define EXCEPTION_TRACE_TOP_LEVEL               ("<top>")

typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    BAD_MULTIBYTE_CHARACTER_IN_COMPILE_ERR,
    CR_IN_REGEXP_ERR,
    CAN_NOT_CREATE_REGEXP_IN_COMPILE_ERR,
    UNEXPECTED_WIDE_STRING_IN_COMPILE_ERR,
    ARRAY_ELEMENT_CAN_NOT_BE_FINAL_ERR,
    COMPLEX_ASSIGNMENT_OPERATOR_TO_FINAL_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef enum {
    IDENTIFIER_NOT_FOUND_ERR = 1,
    ARGUMENT_TOO_MANY_ERR,
    ARGUMENT_TOO_FEW_ERR,
    NOT_BOOLEAN_TYPE_ERR,
    MINUS_OPERAND_TYPE_ERR,
    BAD_OPERAND_TYPE_ERR,
    LOGICAL_OP_DOUBLE_OPERAND_ERR,
    LOGICAL_OP_INTEGER_OPERAND_ERR,
    NOT_BOOLEAN_OPERATOR_ERR,
    NOT_NULL_OPERATOR_ERR,
    NOT_LVALUE_ERR,
    INDEX_OPERAND_NOT_ARRAY_ERR,
    INDEX_OPERAND_NOT_SUPPORT_INDEXING_ERR,
    INDEX_OPERAND_NOT_INT_ERR,
    ARRAY_INDEX_OUT_OF_BOUNDS_ERR,
    ARRAY_SLICE_OUT_OF_BOUNDS_ERR,
    NO_SUCH_METHOD_ERR,
    INC_DEC_OPERAND_TYPE_ERR,
    INC_DEC_OPERAND_NOT_EXIST_ERR,
    NOT_FUNCTION_ERR,
    NOT_OBJECT_MEMBER_UPDATE_ERR,
    NOT_OBJECT_MEMBER_ASSIGN_ERR,
    NO_SUCH_MEMBER_ERR,
    NO_MEMBER_TYPE_ERR,
    BAD_OPERATOR_FOR_STRING_ERR,
    DIVISION_BY_ZERO_ERR,
    GLOBAL_VARIABLE_NOT_FOUND_ERR,
    GLOBAL_STATEMENT_IN_TOPLEVEL_ERR,
    FUNCTION_EXISTS_ERR,
    ARRAY_RESIZE_ARGUMENT_ERR,
    ARRAY_INSERT_ARGUMENT_ERR,
    ARRAY_REMOVE_ARGUMENT_ERR,
    STRING_POS_OUT_OF_BOUNDS_ERR,
    STRING_SUBSTR_LEN_ERR,
    STRING_SUBSTR_ARGUMENT_ERR,
    EXCEPTION_HAS_NO_MESSAGE_ERR,
    EXCEPTION_MESSAGE_IS_NOT_STRING_ERR,
    EXCEPTION_HAS_NO_STACK_TRACE_ERR,
    STACK_TRACE_IS_NOT_ARRAY_ERR,
    STACK_TRACE_LINE_IS_NOT_ASSOC_ERR,
    STACK_TRACE_LINE_HAS_NO_LINE_NUMBER_ERR,
    STACK_TRACE_LINE_HAS_NO_FUNC_NAME_ERR,
    EXCEPTION_IS_NOT_ASSOC_ERR,
    EXCEPTION_HAS_NO_PRINT_STACK_TRACE_METHOD_ERR,
    PRINT_STACK_TRACE_IS_NOT_CLOSURE_ERR,
    BAD_MULTIBYTE_CHARACTER_ERR,
    EXCEPTION_CLASS_IS_NOT_ASSOC_ERR,
    EXCEPTION_CLASS_HAS_NO_CREATE_METHOD_ERR,
    ARGUMENT_TYPE_MISMATCH_ERR,
    UNEXPECTED_WIDE_STRING_ERR,
    ONIG_SEARCH_FAIL_ERR,
    GROUP_INDEX_OVERFLOW_ERR,
    NO_SUCH_GROUP_INDEX_ERR,
    BREAK_OR_CONTINUE_REACHED_TOPLEVEL_ERR,
    ASSIGN_TO_FINAL_VARIABLE_ERR,
    FUNCTION_NOT_FOUND_ERR,
    MODULE_NOT_FOUND_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;


// expression
typedef struct Expression_tag Expression;

typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
//    REGEXP_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    COMMA_EXPRESSION,
    ASSIGN_EXPRESSION,
    CONCAT_STRING_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    LOGICAL_NOT_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    NULL_EXPRESSION,
    ARRAY_EXPRESSION,
    ASSOC_EXPRESSION,
    INDEX_EXPRESSION,
    SLICE_EXPRESSION,
    INCREMENT_EXPRESSION,
    DECREMENT_EXPRESSION,
    CLOSURE_EXPRESSION,
    EXPRESSION_TYPE_COUNT_PLUS_1
} ExpressionType;

#define crb_is_numeric_type(type) ((type) == CRB_INT_VALUE || (type) == CRB_FLOAT_VALUE)
#define crb_is_math_operator(operator) ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION || (operator) == MOD_EXPRESSION)
#define crb_is_compare_operator(operator) ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION || (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION || (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)
#define crb_is_logical_operator(operator) ((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)


typedef LLIST ArgumentList;  // LLIST<Expression*>
//typedef struct ArgumentList_tag {
//    Expression *expression;
//    struct ArgumentList_tag *next;
//} ArgumentList;

typedef struct {
    Expression  *left;
    Expression  *right;
} CommaExpression;

typedef enum {
    NORMAL_ASSIGN = 1,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN
} AssignmentOperator;

typedef struct {
    CRB_Boolean is_final;
    AssignmentOperator  operator;
    Expression  *left;
    Expression  *operand;
} AssignExpression;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

typedef struct {
    Expression          *function;
    ArgumentList        *argument;
} FunctionCallExpression;

typedef void FakeMethodFunc(CRB_Interpreter *inter, CRB_LocalEnvironment *env, CRB_Object *obj, int arg_count, CRB_Value* args, CRB_Value *result);

typedef struct {
    ObjectType  type;
    const char  *name;
    int         param_count;
    FakeMethodFunc* func;
} FakeMethodDefinition;

//typedef struct ExpressionList_tag {
//    ExpressionListNode head;
//    ExpressionListNode* tail;
//} ExpressionList;
typedef LLIST ExpressionList;  // LLIST<Expression*>

// array[index]
typedef struct {
    Expression  *array;  // or string
    Expression  *index;
} IndexExpression;

// array[start:end]
typedef struct {
    Expression  *array;
    Expression  *start;
    Expression  *end;
} SliceExpression;

// expression.member_name
typedef struct {
    Expression          *expression;
    const char          *member_name;
} MemberExpression;

// member_name: expression
typedef struct {
    const char          *member_name;
    Expression          *expression;
    CRB_Boolean         is_final;
} AssocExpression;

typedef LLIST AssocExpressionList;  // LLIST<AssocExpression*>

typedef struct {
    Expression  *operand;
} IncrementOrDecrement;

typedef struct {
    CRB_FunctionDefinition *function_definition;
} ClosureExpression;

//struct CRB_Regexp_tag {
//    CRB_Boolean is_literal;
//    regex_t     *regexp;
//    struct CRB_Regexp_tag *next;
//};

struct Expression_tag {
    ExpressionType type;
    int line_number;
    union {
        CRB_Boolean             boolean_value;
        long                    int_value;
        double                  float_value;
        CRB_Char                *string_value;
//        CRB_Regexp              *regexp_value;
        const char              *identifier;
        CommaExpression         comma;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        Expression              *logical_not;
        FunctionCallExpression  function_call_expression;
        MemberExpression        member_expression;
        ExpressionList          *array_literal;
        AssocExpressionList     *assoc_literal;
        IndexExpression         index_expression;
        SliceExpression         slice_expression;
        IncrementOrDecrement    inc_dec;
        ClosureExpression       closure;
    } u;
};


// statement
typedef enum {
    EXPRESSION_STATEMENT = 1,
    GLOBAL_STATEMENT,
    RETURN_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    FOREACH_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    TRY_STATEMENT,
    THROW_STATEMENT,
    IMPORT_STATEMENT,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

typedef struct Statement_tag Statement;

typedef LLIST StatementList;  // LLIST<Statement*>
//typedef struct StatementList_tag {
//    Statement   *statement;
//    struct StatementList_tag    *next;
//} StatementList;

struct CRB_Block_tag {
    StatementList       *statement_list;
};

typedef LLIST IdentifierList;  // LLIST<char*>
//typedef struct IdentifierList_tag {
//    char        *name;
//    struct IdentifierList_tag   *next;
//} IdentifierList;

typedef struct {
    IdentifierList      *identifier_list;
} GlobalStatement;

typedef struct {
    Expression  *condition;
    CRB_Block   *block;
} Elif;

typedef LLIST ElifList;  // LLIST<Elif*>

typedef struct {
    Expression  *condition;
    CRB_Block   *then_block;
    ElifList    *elif_list;
    CRB_Block   *else_block;
} IfStatement;

typedef struct {
    const char  *label;
    Expression  *condition;
    CRB_Block   *block;
} WhileStatement;

typedef struct {
    const char  *label;
    Expression  *init;
    Expression  *condition;
    Expression  *post;
    CRB_Block   *block;
} ForStatement;

typedef struct {
    const char  *label;
    const char  *variable;
    Expression  *collection;
    CRB_Block   *block;
} ForeachStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

typedef struct {
    const char *label;
} BreakStatement;

typedef struct {
    const char *label;
} ContinueStatement;

typedef struct {
    CRB_Block   *try_block;
    CRB_Block   *catch_block;
    const char  *exception;
    CRB_Block   *finally_block;
} TryStatement;

typedef struct {
    Expression  *exception;
} ThrowStatement;

typedef struct {
    Expression *name;
} ImportStatement;

struct Statement_tag {
    StatementType       type;
    int                 line_number;
    union {
        Expression*     expression_s;
        GlobalStatement global_s;
        IfStatement     if_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        ForeachStatement        foreach_s;
        BreakStatement  break_s;
        ContinueStatement       continue_s;
        ReturnStatement return_s;
        TryStatement    try_s;
        ThrowStatement  throw_s;
        ImportStatement import_s;
    } u;
};



// interpreter
typedef enum {
    CRB_FILE_INPUT_MODE = 1,
    CRB_STRING_INPUT_MODE,
    CRB_READLINE_INPUT_MODE
} CRB_InputMode;

//typedef struct GlobalVariableRef_tag {
//    char        *name;
//    Variable    *variable;
//    struct GlobalVariableRef_tag *next;
//} GlobalVariableRef;
typedef RBTREE GlobalVariableRef;  // RBTREE<Variable*>

typedef struct RefInNativeFunc_tag {
    CRB_Object  *object;
    struct RefInNativeFunc_tag *next;
} RefInNativeFunc;

// 函数的执行环境：上下文（Context）
struct CRB_LocalEnvironment_tag {
    CRB_Module          *module;
    const char          *current_function_name;
    int                 caller_line_number;
    CRB_Object          *scope_chain;      /* ScopeChain 作用域链，每次函数嵌套定义将追加作用域 */
    GlobalVariableRef   *global_var_refs;  // RBTREE<Variable*>
    RefInNativeFunc     *ref_in_native_method;
    struct CRB_LocalEnvironment_tag     *next;
};

typedef struct {
    int         stack_alloc_size;
    int         stack_pointer;
    CRB_Value   *stack;
} Stack;

typedef struct {
    int         current_heap_size;
    int         current_threshold;
    CRB_Object  *header;
} Heap;

typedef struct {
    jmp_buf     environment;
} RecoveryEnvironment;

typedef enum {
    EUC_ENCODING = 1,
    UTF_8_ENCODING
} Encoding;

struct CRB_Interpreter_tag {
    MEM_Storage         interpreter_storage;
    MEM_Storage         execute_storage;
    RBTREE*             modules;  // RBTREE<CRB_Module*>
    RBTREE*             global_vars;  // RBTREE<Variable*>
    RBTREE*             global_funcs;  // RBTREE<CRB_FunctionDefinition*>
    RBTREE*             fake_methods;  // RBTREE<FakeMethodDefinition*>
    StatementList*      statement_list;
    struct lnode*       last_statement_pos;
    CRB_Module*         current_module;
    int                 current_line_number;
    Stack               stack;
    Heap                heap;
    CRB_LocalEnvironment* top_environment;
    CRB_Value           current_exception;
    RecoveryEnvironment current_recovery_environment;
    CRB_InputMode       input_mode;
//    CRB_Regexp          *regexp_literals;
    Encoding            source_encoding;
};

CRB_Interpreter *crb_get_current_interpreter(void);
void crb_set_current_interpreter(CRB_Interpreter *inter);
int _crb_asc_order_named_item(VALUE a, VALUE b);

// others
typedef struct {
    CRB_Char    *string;
    long        len;
    long        cap;
} VString;

#endif //TLANG_TINTERPRETER_H
