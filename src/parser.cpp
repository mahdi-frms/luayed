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
    TokenKind accept[] = {
        TokenKind::Nil,
        TokenKind::True,
        TokenKind::False,
        TokenKind::Number,
        TokenKind::Literal,
        TokenKind::DotDotDot,
        TokenKind::Identifier,
    };
    for (int i = 0; i < sizeof(accept) / sizeof(TokenKind); i++)
    {
        if (accept[i] == kind)
        {
            return true;
        }
    }
    return false;
}
uint8_t check_postfix(TokenKind kind)
{
    if (kind == TokenKind::LeftBrace ||
        kind == TokenKind::LeftParen ||
        kind == TokenKind::LeftBracket ||
        kind == TokenKind::Dot ||
        kind == TokenKind::Literal)
    {
        return 35;
    }
    return 255;
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

Token Parser::consume(TokenKind kind)
{
    Token t = this->pop();
    if (t.kind != kind)
    {
        throw string("expected '") + token_kind_stringify(kind) + "'";
    }
    return t;
}

Noderef Parser::id_field()
{
    Token id = this->pop();
    this->consume(TokenKind::Equal);
    return make_id_field(id, this->expr());
}

Noderef Parser::expr_field()
{
    this->pop(); // [
    Noderef field = this->expr();
    this->consume(TokenKind::RightBracket);
    this->consume(TokenKind::Equal);
    return make_expr_field(field, this->expr());
}

Token Parser::ahead()
{
    Token t = this->tokens[1];
    if (t.kind == TokenKind::Error)
    {
        throw t.text;
    }
    return t;
}

Noderef Parser::table()
{
    vector<Noderef> items;
    while (true)
    {
        Token t = this->peek();
        if (t.kind == TokenKind::RightBrace)
        {
            this->pop();
            break;
        }
        else if (t.kind == TokenKind::Identifier)
        {
            if (this->ahead().kind == TokenKind::Equal)
            {
                items.push_back(this->id_field());
            }
            else
            {
                items.push_back(this->expr());
            }
        }
        else if (t.kind == TokenKind::LeftBracket)
        {
            items.push_back(this->expr_field());
        }
        else
        {
            items.push_back(this->expr());
            // throw string("missing symbol '}'");
        }

        t = this->peek();
        if (t.kind != TokenKind::Comma &&
            t.kind != TokenKind::Semicolon &&
            t.kind != TokenKind::RightBrace)
        {
            throw string("expected ',' or ';'");
        }
        else if (t.kind != TokenKind::RightBrace)
        {
            this->pop();
        }
    }
    return make_table(std::move(items));
}

Noderef Parser::arglist()
{
    vector<Noderef> args;
    while (this->peek().kind != TokenKind::RightParen)
    {
        args.push_back(this->expr());
        if (this->peek().kind == TokenKind::RightParen)
        {
            break;
        }
        this->consume(TokenKind::Comma);
    }
    this->pop();
    return make_arglist(args);
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
    else if (t.kind == TokenKind::LeftBrace)
    {
        lhs = this->table();
    }
    else if (t.kind == TokenKind::LeftParen)
    {
        lhs = this->expr();
        this->consume(TokenKind::RightParen);
    }
    else
    {
        throw string("expression expected");
    }

    while (true)
    {
        Token op = this->peek();
        uint8_t p = check_postfix(op.kind);
        if (p != 255)
        {
            if (t.kind == TokenKind::Identifier || t.kind == TokenKind::RightParen)
                break;
            if (p < pwr)
                break;
            this->pop();
            if (op.kind == TokenKind::Dot)
            {
                Token field = this->consume(TokenKind::Identifier);
                lhs = make_property(lhs, field);
            }
            else if (op.kind == TokenKind::LeftBracket)
            {
                Noderef rhs = this->expr();
                this->consume(TokenKind::RightBracket);
                lhs = make_index(lhs, rhs);
            }
            else if (op.kind == TokenKind::Literal)
            {
                Noderef rhs = make_primary(op);
                lhs = make_call(lhs, rhs);
            }
            else if (op.kind == TokenKind::LeftBrace)
            {
                Noderef rhs = this->table();
                lhs = make_call(lhs, rhs);
            }
            else // left parenthese
            {
                Noderef rhs = this->arglist();
                lhs = make_call(lhs, rhs);
            }
        }
        else
        {
            uint16_t p = check_binary(op.kind);
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
            lhs = make_binary(lhs, rhs, op);
        }
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