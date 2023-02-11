#ifndef LERROR_h
#define LERROR_h

#include <stddef.h>
#include "token.h"

struct Lerror
{
    size_t line = 0;
    size_t offset = 0;
    enum
    {
        // Lexer
        LE_OK,
        LE_MisingEndOfString,
        LE_MissingChar,
        LE_InvalidChar,
        LE_InvalidEscape,
        LE_MalformedNumber,
        LE_MissingEndOfComment,
        // Parser
        LE_ExpectedToken,
        LE_ExpectedExpression,
        LE_ExpectedVariable,
        // Resolver
        LE_VargsOutsideFunction,
        LE_BreakOutsideLoop,
        LE_LabelUndefined,
        LE_LabelRedefined,
        // Runtime
        LE_CallNonFunction,
        LE_NotEnoughArgs,
        LE_IllegalIndex,
        LE_NilIndex,
        // Interpretor
        LE_InvalidOperand,
        LE_InvalidComparison,
        LE_IntegerRepresentation,
    } kind;
    union
    {
        struct
        {
        } ok;

        struct
        {
            size_t level;
        } missing_end_of_string;

        struct
        {
            char c;
        } missing_char;

        struct
        {
            char c;
        } invalid_char;

        struct
        {
        } invalid_escape;

        struct
        {
        } malformed_number;

        struct
        {
            size_t level;
        } missing_end_of_comment;

        struct
        {
            TokenKind token_kind;
        } expected_token;

        struct
        {
        } expected_expression;

        struct
        {
        } expected_variable;

        struct
        {
            LuaType t;
        } invalid_operand;

        struct
        {
            LuaType t1;
            LuaType t2;
        } invalid_comparison;

        struct
        {
        } vargs_outside_function;

        struct
        {
        } breake_outside_loop;

        struct
        {
        } label_undefined;

        struct
        {
            size_t line;
            size_t offset;
        } label_redefined;

        struct
        {
            LuaType t;
        } call_non_function;

        struct
        {
            size_t available;
            size_t expected;
        } not_enough_args;

        struct
        {
        } nil_index;

        struct
        {
            LuaType t;
        } illegal_index;

        struct
        {
        } integer_representation;

    } as;
};

Lerror error_ok();
Lerror error_expected_expression();
Lerror error_expected_variable();
Lerror error_missing_end_of_string(size_t level);
Lerror error_missing_char(char c);
Lerror error_invalid_char(char c);
Lerror error_invalid_escape();
Lerror error_malformed_number();
Lerror error_missing_end_of_comment(size_t level);
Lerror error_expected_token(TokenKind kind);
Lerror error_expected_expression();
Lerror error_expected_variable();
Lerror error_invalid_operand(LuaType t);
Lerror error_invalid_comparison(LuaType t1, LuaType t2);
Lerror error_vargs_outside_function();
Lerror error_breake_outside_loop();
Lerror error_label_undefined();
Lerror error_label_redefined(size_t line, size_t offset);
Lerror error_call_non_function(LuaType t);
Lerror error_not_enough_args(size_t available, size_t expected);
Lerror error_nil_index();
Lerror error_illegal_index(LuaType t);
Lerror error_integer_representation();

#endif