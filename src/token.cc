#include "token.h"
#include <cstring>

using namespace luayed;

Token::Token(size_t ptr, size_t len, size_t line, size_t offset, TokenKind kind)
{
    this->ptr = ptr;
    this->len = len;
    this->line = line;
    this->offset = offset;
    this->kind = kind;
}
Token luayed::token_none()
{
    return Token(0, 0, 0, 0, TokenKind::None);
}

string Token::text(const char *source) const
{
    string str = string(this->len, 0);
    memcpy((void *)str.c_str(), source + this->ptr, this->len);
    return str;
}