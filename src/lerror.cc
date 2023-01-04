#include "lerror.h"
#include "lstrep.h"
#include <iostream>

Lerror error_ok()
{
    Lerror err;
    err.kind = Lerror::LE_OK;
    return err;
}
Lerror error_missing_end_of_string(size_t level)
{
    Lerror err;
    err.kind = Lerror::LE_MisingEndOfString;
    err.as.missing_end_of_string.level = level;
    return err;
}
Lerror error_missing_char(char c)
{
    Lerror err;
    err.kind = Lerror::LE_MissingChar;
    err.as.missing_char.c = c;
    return err;
}
Lerror error_invalid_char(char c)
{
    Lerror err;
    err.kind = Lerror::LE_InvalidChar;
    err.as.invalid_char.c = c;
    return err;
}
Lerror error_invalid_escape()
{
    Lerror err;
    err.kind = Lerror::LE_InvalidEscape;
    return err;
}
Lerror error_malformed_number()
{
    Lerror err;
    err.kind = Lerror::LE_MalformedNumber;
    return err;
}
Lerror error_missing_end_of_comment(size_t level)
{
    Lerror err;
    err.kind = Lerror::LE_MissingEndOfComment;
    err.as.missing_end_of_comment.level = level;
    return err;
}
Lerror error_expected_token(TokenKind kind)
{
    Lerror err;
    err.kind = Lerror::LE_ExpectedToken;
    err.as.expected_token.token_kind = kind;
    return err;
}
Lerror error_expected_expression()
{
    Lerror err;
    err.kind = Lerror::LE_ExpectedExpression;
    return err;
}
Lerror error_expected_variable()
{
    Lerror err;
    err.kind = Lerror::LE_ExpectedVariable;
    return err;
}
Lerror error_invalid_operand(LuaType t)
{
    Lerror err;
    err.kind = Lerror::LE_InvalidOperand;
    err.as.invalid_operand.t = t;
    return err;
}
Lerror error_invalid_comparison(LuaType t1, LuaType t2)
{
    Lerror err;
    err.kind = Lerror::LE_InvalidComparison;
    err.as.invalid_comparison.t1 = t1;
    err.as.invalid_comparison.t2 = t2;
    return err;
}
Lerror error_vargs_outside_function()
{
    Lerror err;
    err.kind = Lerror::LE_VargsOutsideFunction;
    return err;
}
Lerror error_breake_outside_loop()
{
    Lerror err;
    err.kind = Lerror::LE_BreakOutsideLoop;
    return err;
}
Lerror error_label_undefined()
{

    Lerror err;
    err.kind = Lerror::LE_LabelUndefined;
    return err;
}
Lerror error_label_redefined(size_t line, size_t offset)
{
    Lerror err;
    err.kind = Lerror::LE_LabelRedefined;
    err.as.label_redefined.line = line;
    err.as.label_redefined.offset = offset;
    return err;
}