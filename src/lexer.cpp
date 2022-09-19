#include "lexer.hpp"

#define RET(T)                           \
    {                                    \
        auto tmp = T;                    \
        if (tmp.kind != TokenKind::None) \
            return tmp;                  \
    }                                    \
    0

TokenKind
single_op(char c)
{
    if (c == '+')
        return TokenKind::Plus;
    if (c == '^')
        return TokenKind::Power;
    if (c == '*')
        return TokenKind::Multiply;
    if (c == '#')
        return TokenKind::Length;
    if (c == '%')
        return TokenKind::Modulo;
    if (c == '&')
        return TokenKind::BinAnd;
    if (c == '|')
        return TokenKind::BinOr;
    if (c == '(')
        return TokenKind::RightParen;
    if (c == ')')
        return TokenKind::LeftParen;
    if (c == '{')
        return TokenKind::LeftBrace;
    if (c == '}')
        return TokenKind::RightBrace;
    if (c == ';')
        return TokenKind::Semicolon;
    if (c == ',')
        return TokenKind::Comma;
    return TokenKind::None;
}

bool strinclues(const char *str, char c)
{
    const char *ptr = str;
    while (*ptr != '\0')
    {
        if (*ptr == c)
            return true;
    }
    return false;
}

char Lexer::read()
{
    return this->text[this->pos++];
}

char Lexer::peek()
{
    return this->text[this->pos];
}

Lexer::Lexer(string &text) : text(text)
{
    this->pos = 0;
    this->line = 0;
    this->offset = 0;
    this->tokens = vector<Token>();
}

Token::Token(string text, size_t line, size_t offset, TokenKind kind)
{
    this->kind = kind;
    this->offset = offset;
    this->line = line;
    this->text = text;
}

Token Lexer::next()
{
    size_t pos = this->pos;
    Token token = this->pop();
    for (size_t i = pos; i < this->pos; i++)
    {
        if (this->text[i] == '\n')
        {
            this->line++;
            this->offset = 0;
        }
        else
        {
            this->offset++;
        }
    }
    return token;
}

Token Lexer::pop()
{
    if (this->peek() == '\0')
        return this->token("", TokenKind::Eof);
    char c = this->read();
    while (c == ' ' || c == '\n')
    {
        c = this->read();
    }
    TokenKind tk = single_op(c);
    if (tk != TokenKind::None)
    {
        return this->token(string(1, c), tk);
    }
    RET(this->op_dot(c));
    RET(this->op_equal(c));
    RET(this->op_equal(c));
    RET(this->op_less(c));
    RET(this->op_negate(c));
    RET(this->op_divide(c));
    if (c == '-')
    {
        if (this->peek() == '-')
        {
            this->read();
            if (this->peek() == '[')
            {
                this->read();
                if (this->peek() == '[')
                {
                    this->read();
                    // read all comments
                }
                else
                {
                    this->skip_line();
                }
            }
            else
            {
                this->skip_line();
            }
        }
        else
            return this->token(string("-"), TokenKind::Minus);
    }
}

vector<Token> Lexer::drain()
{
    while (true)
    {
        Token t = this->next();
        this->tokens.push_back(t);
        if (t.kind == TokenKind::Eof)
            break;
    }
    return std::move(this->tokens);
}

Token Lexer::token(string text, TokenKind kind)
{
    return Token(text, this->line, this->offset, kind);
}

void Lexer::skip_line()
{
    char c = this->peek();
    while (c != '\n' && c != '\0')
        this->read();
}

Token Lexer::token_eof()
{
    return this->token(string(""), TokenKind::Eof);
}

Token Lexer::op_equal(char c)
{
    if (c == '=')
    {
        if (this->peek() == '=')
        {
            read();
            return this->token(string("=="), TokenKind::EqualEqual);
        }
        else
            return this->token(string("="), TokenKind::Equal);
    }
    else
        return this->token_eof();
}
Token Lexer::op_negate(char c)
{
    if (c == '~')
    {
        if (this->peek() == '=')
        {
            read();
            return this->token(string("~="), TokenKind::NotEqual);
        }
        else
            return this->token(string("~"), TokenKind::Negate);
    }
    else
        return this->token_eof();
}
Token Lexer::op_dot(char c)
{
    if (c == '.')
    {
        if (this->peek() == '.')
        {
            this->read();
            if (this->peek() == '.')
            {
                return this->token(string("..."), TokenKind::DotDotDot);
            }
            else
            {
                return this->token(string(".."), TokenKind::DotDot);
            }
        }
        else
            return this->token(string("."), TokenKind::Dot);
    }
    else
        return this->token_eof();
}
Token Lexer::op_divide(char c)
{
    if (c == '/')
    {
        if (this->peek() == '/')
        {
            read();
            return this->token(string("//"), TokenKind::FloorDivision);
        }
        else
            return this->token(string("/"), TokenKind::FloatDivision);
    }
    else
        return this->token_eof();
}
Token Lexer::op_less(char c)
{
    if (c == '<')
    {
        if (this->peek() == '<')
        {
            read();
            return this->token(string("<<"), TokenKind::LeftShift);
        }
        else if (this->peek() == '=')
        {
            read();
            return this->token(string("<="), TokenKind::LessEqual);
        }
        else
            return this->token(string("<"), TokenKind::Less);
    }
    else
        return this->token_eof();
}
Token Lexer::op_greater(char c)
{
    if (c == '>')
    {
        if (this->peek() == '>')
        {
            read();
            return this->token(string(">>"), TokenKind::RightShift);
        }
        else if (this->peek() == '=')
        {
            read();
            return this->token(string(">="), TokenKind::GreaterEqual);
        }
        else
            return this->token(string(">"), TokenKind::Greater);
    }
    else
        return this->token_eof();
}