#include "token.h"
#include <cstring>

Token::Token(const char *str, size_t len, size_t line, size_t offset, TokenKind kind)
{
    this->str = str;
    this->len = len;
    this->line = line;
    this->offset = offset;
    this->kind = kind;
}
Token token_none()
{
    return Token(nullptr, 0, 0, 0, TokenKind::None);
}

string Token::text() const
{
    string str = string(this->len, 0);
    memcpy((void *)str.c_str(), this->str, this->len);
    return str;
}