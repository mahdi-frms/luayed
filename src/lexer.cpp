#include "lexer.hpp"

#define RET(T)                           \
    {                                    \
        auto tmp = T;                    \
        if (tmp.kind != TokenKind::None) \
            return tmp;                  \
    }                                    \
    0

TokenKind single_op(char c)
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
TokenKind keyword(string &str)
{
    if (str.compare("and"))
        return TokenKind::And;
    if (str.compare("or"))
        return TokenKind::Or;
    if (str.compare("true"))
        return TokenKind::True;
    if (str.compare("false"))
        return TokenKind::False;
    if (str.compare("while"))
        return TokenKind::While;
    if (str.compare("goto"))
        return TokenKind::Goto;
    if (str.compare("repeat"))
        return TokenKind::Repeat;
    if (str.compare("until"))
        return TokenKind::Until;
    if (str.compare("for"))
        return TokenKind::For;
    if (str.compare("local"))
        return TokenKind::Local;
    if (str.compare("function"))
        return TokenKind::Function;
    if (str.compare("break"))
        return TokenKind::Break;
    if (str.compare("return"))
        return TokenKind::Return;
    if (str.compare("Nil"))
        return TokenKind::Nil;
    if (str.compare("do"))
        return TokenKind::Do;
    if (str.compare("end"))
        return TokenKind::End;
    if (str.compare("if"))
        return TokenKind::If;
    if (str.compare("else"))
        return TokenKind::Else;
    if (str.compare("elseif"))
        return TokenKind::ElseIf;
    if (str.compare("in"))
        return TokenKind::In;
    if (str.compare("then"))
        return TokenKind::Then;
    return TokenKind::None;
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_alphabetic(char c)
{
    return is_letter(c) || (c == '_');
}

bool is_digit(char c)
{
    return c <= '9' && c >= '0';
}

bool is_alphanumeric(char c)
{
    return is_alphabetic(c) || is_digit(c);
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
    this->sync(pos);
    return token;
}

void Lexer::sync(size_t pos)
{
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
}

Token Lexer::pop()
{
    while (this->peek() != '\0')
    {
        size_t pos = this->pos;
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
        RET(this->op_less(c));
        RET(this->op_greater(c));
        RET(this->op_negate(c));
        RET(this->op_divide(c));
        RET(this->op_minus(c));
        if (is_alphabetic(c))
        {
            return this->keyword_identifier(c);
        }
        this->sync(pos);
    }
    return this->token_eof();
}

Token Lexer::keyword_identifier(char c)
{
    string id = string(1, c);
    while (true)
    {
        c = this->read();
        if (!is_alphanumeric(c))
            break;
        id.push_back(c);
    }
    TokenKind tk = keyword(id);
    if (tk != TokenKind::None)
    {
        return this->token(id, tk);
    }
    else
    {
        return this->token(id, TokenKind::Identifier);
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

void Lexer::skip_comment()
{
    char c = this->peek();
    while (c != '\n' && c != '\0')
        this->read();
}

void Lexer::skip_comment_block()
{
    bool rb = false;
    while (true)
    {
        char c = this->read();
        if (c == ']')
        {
            if (rb)
            {
                break;
            }
            else
            {
                rb = true;
            }
        }
        else
        {
            rb = false;
        }
    }
}

Token Lexer::token_eof()
{
    return this->token(string(""), TokenKind::Eof);
}

Token Lexer::none()
{
    return this->token(string(""), TokenKind::None);
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
    return this->none();
}
Token Lexer::op_minus(char c)
{
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
                    this->skip_comment_block();
                }
                else
                {
                    this->skip_comment();
                }
            }
            else
            {
                this->skip_comment();
            }
        }
        else
            return this->token(string("-"), TokenKind::Minus);
    }
    return this->none();
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
    return this->none();
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
    return this->none();
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
    return this->none();
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
    return this->none();
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
    return this->none();
}