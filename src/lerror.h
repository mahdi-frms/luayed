#ifndef LERROR_h
#define LERROR_h

#include <stddef.h>
#include "token.h"

struct LError
{
    size_t line;
    size_t offset;
    enum
    {
        LE_OK,
        LE_MisingEndOfString,
        LE_MissingChar,
        LE_InvalidChar,
        LE_InvalidEscape,
        LE_MalformedNumber,
        LE_MissingEndOfComment,
        LE_ExpectedToken,
        LE_ExpectedExpression,
        LE_ExpectedVariable,
        LE_InvalidBinaryOperands,
        LE_InvalidUnaryOperand,

        LE_VargsOutsideFunction,
        LE_BreakOutsideLoop,
        LE_LabelUndefined,
        LE_LabelRedefined,
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
            LuaType t1;
            LuaType t2;
        } invalid_binary_operands;
        struct
        {
            LuaType t;
        } invalid_unary_operands;
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
    } as;
};

LError error_ok();
LError error_expected_expression();
LError error_expected_variable();
LError error_missing_end_of_string(size_t level);
LError error_missing_char(char c);
LError error_invalid_char(char c);
LError error_invalid_escape();
LError error_malformed_number();
LError error_missing_end_of_comment(size_t level);
LError error_expected_token(TokenKind kind);
LError error_expected_expression();
LError error_expected_variable();
LError error_invalid_binary_operands(LuaType t1, LuaType t2);
LError error_invalid_unary_operand(LuaType t);
LError error_vargs_outside_function();
LError error_breake_outside_loop();
LError error_label_undefined();
LError error_label_redefined(size_t line, size_t offset);

#endif