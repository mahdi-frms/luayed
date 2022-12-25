#include "token.h"
#include <cstring>

string token_kind_stringify(TokenKind kind)
{
    if (kind == TokenKind::And)
        return "and";
    if (kind == TokenKind::BinAnd)
        return "binary and";
    if (kind == TokenKind::BinOr)
        return "binary or";
    if (kind == TokenKind::Break)
        return "break";
    if (kind == TokenKind::Colon)
        return "colon";
    if (kind == TokenKind::ColonColon)
        return "colon colon";
    if (kind == TokenKind::Comma)
        return "comma";
    if (kind == TokenKind::Do)
        return "do";
    if (kind == TokenKind::Dot)
        return "dot";
    if (kind == TokenKind::DotDot)
        return "dot dot";
    if (kind == TokenKind::DotDotDot)
        return "dot dot dot";
    if (kind == TokenKind::Else)
        return "else";
    if (kind == TokenKind::ElseIf)
        return "else if";
    if (kind == TokenKind::End)
        return "end";
    if (kind == TokenKind::Eof)
        return "EOF";
    if (kind == TokenKind::Equal)
        return "equal";
    if (kind == TokenKind::EqualEqual)
        return "equal equal";
    if (kind == TokenKind::Error)
        return "!error";
    if (kind == TokenKind::Empty)
        return "_empty_";
    if (kind == TokenKind::False)
        return "false";
    if (kind == TokenKind::FloatDivision)
        return "float division";
    if (kind == TokenKind::FloorDivision)
        return "floor division";
    if (kind == TokenKind::For)
        return "for";
    if (kind == TokenKind::Function)
        return "function";
    if (kind == TokenKind::Goto)
        return "goto";
    if (kind == TokenKind::Greater)
        return "greater";
    if (kind == TokenKind::GreaterEqual)
        return "greater equal";
    if (kind == TokenKind::Identifier)
        return "identifier";
    if (kind == TokenKind::If)
        return "if";
    if (kind == TokenKind::In)
        return "in";
    if (kind == TokenKind::LeftBrace)
        return "left brace";
    if (kind == TokenKind::LeftBracket)
        return "left bracket";
    if (kind == TokenKind::LeftParen)
        return "left parentheses";
    if (kind == TokenKind::LeftShift)
        return "left shift";
    if (kind == TokenKind::Length)
        return "length";
    if (kind == TokenKind::Less)
        return "less";
    if (kind == TokenKind::LessEqual)
        return "less equal";
    if (kind == TokenKind::Literal)
        return "literal";
    if (kind == TokenKind::Local)
        return "local";
    if (kind == TokenKind::Minus)
        return "minus";
    if (kind == TokenKind::Modulo)
        return "modulo";
    if (kind == TokenKind::Multiply)
        return "multiply";
    if (kind == TokenKind::Negate)
        return "negate";
    if (kind == TokenKind::Nil)
        return "nil";
    if (kind == TokenKind::None)
        return "-NONE-";
    if (kind == TokenKind::Not)
        return "not";
    if (kind == TokenKind::NotEqual)
        return "not equal";
    if (kind == TokenKind::Number)
        return "number";
    if (kind == TokenKind::Or)
        return "or";
    if (kind == TokenKind::Plus)
        return "plus";
    if (kind == TokenKind::Power)
        return "power";
    if (kind == TokenKind::Repeat)
        return "repeat";
    if (kind == TokenKind::Return)
        return "return";
    if (kind == TokenKind::RightBrace)
        return "right brace";
    if (kind == TokenKind::RightBracket)
        return "right bracket";
    if (kind == TokenKind::RightParen)
        return "right paren";
    if (kind == TokenKind::RightShift)
        return "right shift";
    if (kind == TokenKind::Semicolon)
        return "semicolon";
    if (kind == TokenKind::Then)
        return "then";
    if (kind == TokenKind::True)
        return "true";
    if (kind == TokenKind::Until)
        return "until";
    return "while";
}
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

string Token::text()
{
    string str = string(this->len, 0);
    memcpy((void *)str.c_str(), this->str, this->len);
    return str;
}