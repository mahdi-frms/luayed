#include "parser.hpp"
#include <utility>
using namespace ast;

uint16_t priorities(uint8_t l, uint8_t r)
{
    return l * 256 + r;
}

uint8_t check_prefix(TokenKind kind)
{
    if (kind == TokenKind::Not || kind == TokenKind::Negate || kind == TokenKind::Minus)
        return 31;
    return 255;
}

Token errtoken()
{
    return Token("", 0, 0, TokenKind::Error);
}

bool is_primary(TokenKind kind)
{
    TokenKind accpet[] = {
        TokenKind::Nil,
        TokenKind::True,
        TokenKind::False,
        TokenKind::Number,
        TokenKind::Literal,
        TokenKind::DotDotDot,
    };
    for (int i = 0; i < sizeof(accpet) / sizeof(TokenKind); i++)
    {
        if (accpet[i] == kind)
        {
            return true;
        }
    }
    return false;
}

uint16_t check_binary(TokenKind kind)
{
    if (kind == TokenKind::Or)
        return priorities(11, 12);
    if (kind == TokenKind::And)
        return priorities(13, 14);
    if (kind == TokenKind::EqualEqual ||
        kind == TokenKind::NotEqual ||
        kind == TokenKind::GreaterEqual ||
        kind == TokenKind::Less ||
        kind == TokenKind::GreaterEqual ||
        kind == TokenKind::LessEqual)
        return priorities(15, 16);
    if (kind == TokenKind::BinOr)
        return priorities(17, 18);
    if (kind == TokenKind::Negate)
        return priorities(19, 20);
    if (kind == TokenKind::BinAnd)
        return priorities(21, 22);
    if (kind == TokenKind::RightShift || kind == TokenKind::LeftShift)
        return priorities(23, 24);
    if (kind == TokenKind::DotDot)
        return priorities(26, 25);
    if (kind == TokenKind::Plus || kind == TokenKind::Minus)
        return priorities(27, 28);
    if (kind == TokenKind::Multiply || kind == TokenKind::FloatDivision || kind == TokenKind::FloorDivision)
        return priorities(29, 30);
    if (kind == TokenKind::Power)
        return priorities(34, 33);
    return (uint16_t)-1;
}

Parser::Parser(Lexer &lexer) : lexer(lexer)
{
    this->tokens = this->lexer.drain();
}

Node::Node(Gnode inner, NodeKind kind) : inner(inner), kind(kind)
{
}

/*
    11 12       or
    13 14       and
    15 16       < > <= >= ~= ==
    17 18       |
    19 20       ~
    21 22       &
    23 24       >> <<
    26 25       ..
    27 28       + -
    29 30       * /
    31          not -(unary) ~(unary)
    34 33       ^
*/
Noderef Parser::expr()
{
    return this->expr_p(0);
}

Noderef Parser::expr_p(uint8_t pwr)
{
    Token t = this->pop();
    uint8_t prefix = check_prefix(t.kind);
    Noderef lhs = nullptr;
    if (prefix != 255)
    {
        Noderef rhs = this->expr_p(prefix);
        lhs = make_unary(rhs, t);
    }
    else if (is_primary(t.kind))
    {
        lhs = make_primary(t);
    }
    else
    {
        throw string("expression expected");
    }

    while (true)
    {
        Token t = this->peek();
        uint16_t p = check_binary(t.kind);
        if (p == (uint16_t)-1)
            break;
        uint8_t lp = p >> 8;
        uint8_t rp = p % 256;

        if (pwr > lp)
        {
            break;
        }
        this->pop();
        Noderef rhs = expr_p(rp);
        lhs = make_binary(lhs, rhs, t);
    }

    return lhs;
}

Token Parser::pop()
{
    Token t = this->tokens.front();
    this->tokens.erase(this->tokens.begin());
    if (t.kind == TokenKind::Error)
    {
        throw t.text;
    }
    return t;
}

Token Parser::peek()
{
    Token t = this->tokens.front();
    if (t.kind == TokenKind::Error)
    {
        throw t.text;
    }
    return t;
}

Ast Parser::parse()
{
    try
    {
        return Ast(this->expr());
    }
    catch (string message)
    {
        printf("lua: %s\n", message.c_str());
        return Ast(nullptr);
    }
}