#include "lerror.h"
#include "lstrep.h"
#include <iostream>

LError error_ok()
{
    LError err;
    err.kind = LError::LE_OK;
    return err;
}
LError error_missing_end_of_string(size_t level)
{
    LError err;
    err.kind = LError::LE_MisingEndOfString;
    err.as.missing_end_of_string.level = level;
    return err;
}
LError error_missing_char(char c)
{
    LError err;
    err.kind = LError::LE_MissingChar;
    err.as.missing_char.c = c;
    return err;
}
LError error_invalid_char(char c)
{
    LError err;
    err.kind = LError::LE_InvalidChar;
    err.as.invalid_char.c = c;
    return err;
}
LError error_invalid_escape()
{
    LError err;
    err.kind = LError::LE_InvalidEscape;
    return err;
}
LError error_malformed_number()
{
    LError err;
    err.kind = LError::LE_MalformedNumber;
    return err;
}
LError error_missing_end_of_comment(size_t level)
{
    LError err;
    err.kind = LError::LE_MissingEndOfComment;
    err.as.missing_end_of_comment.level = level;
    return err;
}
LError error_expected_token(TokenKind kind)
{
    LError err;
    err.kind = LError::LE_ExpectedToken;
    err.as.expected_token.token_kind = kind;
    return err;
}
LError error_expected_expression()
{
    LError err;
    err.kind = LError::LE_ExpectedExpression;
    return err;
}
LError error_expected_variable()
{
    LError err;
    err.kind = LError::LE_ExpectedVariable;
    return err;
}
LError error_invalid_binary_operands(LuaType t1, LuaType t2)
{
    LError err;
    err.kind = LError::LE_InvalidBinaryOperands;
    err.as.invalid_binary_operands.t1 = t1;
    err.as.invalid_binary_operands.t2 = t2;
    return err;
}
LError error_invalid_unary_operand(LuaType t)
{
    LError err;
    err.kind = LError::LE_InvalidUnaryOperand;
    err.as.invalid_unary_operands.t = t;
    return err;
}