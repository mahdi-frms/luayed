#include "lerror.hpp"
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
    err.kind = LError::LE_MissingChar;
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

std::ostream &operator<<(std::ostream &os, const LError &err)
{
    if (err.kind == LError::LE_OK)
        return os;
    os << "lua: error(line: "
       << err.line
       << ", offset: "
       << err.offset
       << "): ";
    if (err.kind == LError::LE_MissingEndOfComment)
    {
        os << "missing symbol ']";
        for (size_t i = 0; i < err.as.missing_end_of_comment.level; i++)
            os << '=';
        os << "]'";
    }
    else if (err.kind == LError::LE_MisingEndOfString)
    {
        os << "missing symbol ']";
        for (size_t i = 0; i < err.as.missing_end_of_string.level; i++)
            os << '=';
        os << "]'";
    }
    else if (err.kind == LError::LE_MissingChar)
    {
        os << "missing character '" << err.as.missing_char.c << "'";
    }
    else if (err.kind == LError::LE_InvalidChar)
    {
        os << "invalid character '" << err.as.invalid_char.c << "'";
    }
    else if (err.kind == LError::LE_InvalidEscape)
    {
        os << "invalid escape";
    }
    else if (err.kind == LError::LE_MalformedNumber)
    {
        os << "malformed number";
    }
    else
    {
        os << "LUA CRASH: ERROR NOT SUPPORTED FOR DISPLAY";
        exit(1);
    }
    os << "\n";
    return os;
}