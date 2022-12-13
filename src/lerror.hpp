#ifndef LERROR_HPP
#define LERROR_HPP

#include <stddef.h>
#include "luadef.hpp"

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
    } as;

    string to_string();
};

LError error_ok();
LError error_missing_end_of_string(size_t level);
LError error_missing_char(char c);
LError error_invalid_char(char c);
LError error_invalid_escape();
LError error_malformed_number();
LError error_missing_end_of_comment(size_t level);

std::ostream &operator<<(std::ostream &os, const LError &err);

#endif