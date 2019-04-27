//
// Created by t5w0rd on 19-4-24.
//

#include "tinterpreter.h"


static const char* st_builtin_src[] = {
        "func create_exception_class(parent) {\n",
            "this = {};\n",
            "this.parent = parent;\n",
            "this.create = closure(message) {\n",
                "e = exception(message);\n",
                "e.stack_trace.pop(0);\n",
                "e.child_of = this.child_of;\n",
                "return e;\n",
            "};\n",
            "this.child_of = closure(o) {\n",
                "for (p=this; p!=null; p=p.parent) {\n",
                    "if (p == o) {\n",
                        "return true;\n",
                    "}\n",
                "}\n",
                "return false;\n",
            "};\n",
            "return this;\n",
        "}\n",
        "\n",
        "RootException = create_exception_class(null);\n",
        "BugException = create_exception_class(RootException);\n",
        "RuntimeException = create_exception_class(RootException);\n",
        "ArithmeticException = create_exception_class(RuntimeException);\n",
        "VariableNotFoundException = create_exception_class(BugException);\n",
        "ArgumentTooManyException = create_exception_class(BugException);\n",
        "ArgumentTooFewException = create_exception_class(BugException);\n",
        "NotBooleanException = create_exception_class(BugException);\n",
        "MinusOperandTypeException = create_exception_class(BugException);\n",
        "BadOperandTypeException = create_exception_class(BugException);\n",
        "LogicalOperatorDoubleOperandException = create_exception_class(BugException);\n",
        "LogicalOperatorIntegerOperandException = create_exception_class(BugException);\n",
        "NotBooleanOperatorException = create_exception_class(BugException);\n",
        "NotNullOperatorException = create_exception_class(BugException);\n",
        "NotLValueException = create_exception_class(BugException);\n",
        "IndexOperandNotArrayException = create_exception_class(BugException);\n",
        "IndexOperandNotSupportIndexingException = create_exception_class(BugException);\n",
        "IndexOperandNotIntException = create_exception_class(BugException);\n",
        "ArrayIndexOutOfBoundsException = create_exception_class(BugException);\n",
        "ArraySliceOutOfBoundsException = create_exception_class(BugException);\n",
        "NoSuchMethodException = create_exception_class(BugException);\n",
        "IncDecOperandTypeException = create_exception_class(BugException);\n",
        "IncDecOperandNotExistException = create_exception_class(BugException);\n",
        "NotFunctionException = create_exception_class(BugException);\n",
        "NotObjectMemberUpdateException = create_exception_class(BugException);\n",
        "NotObjectMemberAssignException = create_exception_class(BugException);\n",
        "NoSuchMemberException = create_exception_class(BugException);\n",
        "NoMemberTypeException = create_exception_class(BugException);\n",
        "BadOperatorForStringException = create_exception_class(BugException);\n",
        "DivisionByZeroException = create_exception_class(ArithmeticException);\n",
        "GlobalVariableNotFoundException  = create_exception_class(BugException);\n",
        "GlobalStatementInToplevelException = create_exception_class(BugException);\n",
        "FunctionExistsException = create_exception_class(BugException);\n",
        "ArrayResizeArgumentException = create_exception_class(BugException);\n",
        "ArrayInsertArgumentException = create_exception_class(BugException);\n",
        "ArrayRemoveArgumentException = create_exception_class(BugException);\n",
        "StringPositionOutOfBoundsException = create_exception_class(BugException);\n",
        "StringSubstrLengthException = create_exception_class(BugException);\n",
        "StringSubstrArgumentException = create_exception_class(BugException);\n",
        "ExceptionHasNoMessageException = create_exception_class(BugException);\n",
        "ExceptionMessageIsNotStringException = create_exception_class(BugException);\n",
        "ExceptionHasNoStackTraceException = create_exception_class(BugException);\n",
        "StackTraceIsNotArrayException = create_exception_class(BugException);\n",
        "StackTraceLineIsNotAssocException = create_exception_class(BugException);\n",
        "StackTraceLineHasNoLineNumberException = create_exception_class(BugException);\n",
        "StackTraceLineHasNoFuncNameException = create_exception_class(BugException);\n",
        "ExceptionIsNotAssocException = create_exception_class(BugException);\n",
        "\n",
        "ExceptionHasNoPrintStackTraceMethodException = create_exception_class(BugException);\n",
        "PrintStackTraceIsNotClosureException = create_exception_class(BugException);\n",
        "BadMultibyteCharacterException = create_exception_class(RuntimeException);\n",
        "ExceptionClassIsNotAssocException = create_exception_class(BugException);\n",
        "ExceptionClassHasNoCreateMethodException = create_exception_class(BugException);\n",
        "ArgumentTypeMismatchException = create_exception_class(BugException);\n",
        "UnexpectedWideStringException = create_exception_class(RuntimeException);\n",
        "OnigSearchFailException = create_exception_class(RuntimeException);\n",
        "GroupIndexOverflowException = create_exception_class(BugException);\n",
        "BreakOrContinueReachedTopLevelException = create_exception_class(BugException);\n",
        "AssignToFinalVariableException = create_exception_class(BugException);\n",
        "FunctionNotFoundException = create_exception_class(BugException);\n",
        "\n",
        "# native.c\n",
        "FOpenArgumentTypeException = create_exception_class(BugException);\n",
        "FCloseArgumentTypeException = create_exception_class(BugException);\n",
        "FGetsArgumentTypeException = create_exception_class(BugException);\n",
        "FileAlreadyClosedException = create_exception_class(BugException);\n",
        "FPutsArgumentTypeException = create_exception_class(BugException);\n",
        "NewArrayArgumentTypeException = create_exception_class(BugException);\n",
        "NewArrayArgumentTooFewException = create_exception_class(BugException);\n",
        "ExitArgumentTypeException = create_exception_class(BugException);\n",
        "NewExceptionArgumentException = create_exception_class(BugException);\n",
        "FGetsBadMultibyteCharacterException = create_exception_class(BadMultibyteCharacterException);\n",
        "\n",
        "# iterator\n",
        "func __create_array_iterator(array) {\n",
            "this = {};\n",
            "index = 0;\n",
            "this.first = closure() {\n",
                "index = 0;\n",
            "};\n",
            "this.next = closure() {\n",
                "index++;\n",
            "};\n",
            "this.is_done = closure() {\n",
                "return index >= array.len();\n",
            "};\n",
            "this.current_item = closure() {\n",
                "return array[index];\n",
            "};\n",
            "return this;\n",
        "}\n",
        NULL};

static const char **st_src_array[] = {
        st_builtin_src,
};

void crb_compile_built_in_script(CRB_Interpreter *inter) {
    for (int i = 0; i < sizeof(st_src_array) / sizeof(st_src_array[0]); i++) {
        CRB_compile_string(inter, st_src_array[i]);
    }
}
