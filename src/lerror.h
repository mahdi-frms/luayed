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
        LE_InvalidOperands,
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
        } invalid_operands;
    } as;

    string to_string();
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
LError error_invalid_operands(LuaType t1, LuaType t2);

std::ostream &operator<<(std::ostream &os, const LError &err);

#endif