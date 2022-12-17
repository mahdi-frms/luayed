#include "lexer.hpp"
#include <cstring>

#define RET(T)                           \
    {                                    \
        auto tmp = T;                    \
        if (tmp.kind != TokenKind::None) \
            return tmp;                  \
    }

#define RETK(T)                     \
    {                               \
        auto tmp = T;               \
        if (tmp != TokenKind::None) \
            return tmp;             \
    }

TokenKind single_op(char c)
{
    if (c == '+')
        return TokenKind::Plus;
    if (c == '^')
        return TokenKind::Power;
    if (c == '*')
        return TokenKind::Multiply;
    if (c == '%')
        return TokenKind::Modulo;
    if (c == '&')
        return TokenKind::BinAnd;
    if (c == '|')
        return TokenKind::BinOr;
    if (c == '(')
        return TokenKind::LeftParen;
    if (c == ')')
        return TokenKind::RightParen;
    if (c == '{')
        return TokenKind::LeftBrace;
    if (c == '}')
        return TokenKind::RightBrace;
    if (c == ']')
        return TokenKind::RightBracket;
    if (c == ';')
        return TokenKind::Semicolon;
    if (c == ',')
        return TokenKind::Comma;
    return TokenKind::None;
}
TokenKind Lexer::kw(const char *str, size_t idx, TokenKind kind)
{
    if (this->pos - this->prev_pos != strlen(str))
        return TokenKind::None;
    for (size_t i = 0; i < this->pos - this->prev_pos - idx; i++)
        if (this->ch(idx + i) != str[idx + i])
            return TokenKind::None;
    return kind;
}
TokenKind Lexer::keyword()
{
    if (this->ch(0) == 'a')
    {
        return this->kw("and", 1, TokenKind::And);
    }
    else if (this->ch(0) == 'b')
    {
        return this->kw("break", 1, TokenKind::Break);
    }
    else if (this->ch(0) == 'd')
    {
        return this->kw("do", 1, TokenKind::Do);
    }
    else if (this->ch(0) == 'e')
    {
        RETK(this->kw("end", 1, TokenKind::End));
        RETK(this->kw("else", 1, TokenKind::Else));
        return this->kw("elseif", 1, TokenKind::ElseIf);
    }
    else if (this->ch(0) == 'f')
    {
        RETK(this->kw("for", 1, TokenKind::For));
        RETK(this->kw("false", 1, TokenKind::False));
        return this->kw("function", 1, TokenKind::Function);
    }
    else if (this->ch(0) == 'g')
    {
        return this->kw("goto", 1, TokenKind::Goto);
    }
    else if (this->ch(0) == 'i')
    {
        RETK(this->kw("in", 1, TokenKind::In));
        return this->kw("if", 1, TokenKind::If);
    }
    else if (this->ch(0) == 'l')
    {
        return this->kw("local", 1, TokenKind::Local);
    }
    else if (this->ch(0) == 'n')
    {
        RETK(this->kw("not", 1, TokenKind::Not));
        return this->kw("nil", 1, TokenKind::Nil);
    }
    else if (this->ch(0) == 'o')
    {
        return this->kw("or", 1, TokenKind::Or);
    }
    else if (this->ch(0) == 'r')
    {
        RETK(this->kw("return", 1, TokenKind::Return));
        return this->kw("repeat", 1, TokenKind::Repeat);
    }
    else if (this->ch(0) == 't')
    {
        RETK(this->kw("then", 1, TokenKind::Then));
        return this->kw("true", 1, TokenKind::True);
    }
    else if (this->ch(0) == 'u')
    {
        return this->kw("until", 1, TokenKind::Until);
    }
    else if (this->ch(0) == 'w')
    {
        return this->kw("while", 1, TokenKind::While);
    }
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

bool is_hex(char c)
{
    return (c <= '9' && c >= '0') || (c <= 'f' && c >= 'a') || (c <= 'F' && c >= 'A');
}

bool is_alphanumeric(char c)
{
    return is_alphabetic(c) || is_digit(c);
}

Token Lexer::empty()
{
    return Token(nullptr, 0, 0, 0, TokenKind::Empty);
}

char Lexer::pop()
{
    char c = this->text[this->pos];
    if (c == '\n')
    {
        this->offset = 0;
        this->line++;
    }
    else if (c == '\t')
    {
        this->offset += 4;
    }
    else
    {
        this->offset++;
    }
    this->pos++;
    return c;
}

char Lexer::peek()
{
    return this->text[this->pos];
}

Lexer::Lexer(const char *text) : text(text)
{
    this->pos = 0;
    this->line = 0;
    this->offset = 0;
    this->prev_pos = 0;
    this->prev_line = 0;
    this->prev_offset = 0;
    this->err = error_ok();
}

Token Lexer::next()
{
    Token token = this->read();
    while (token.kind == TokenKind::Empty)
    {
        this->sync();
        token = this->read();
    }
    this->sync();
    return token;
}

void Lexer::sync()
{
    this->prev_line = this->line;
    this->prev_offset = this->offset;
    this->prev_pos = this->pos;
}

bool is_space(char c)
{
    return c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\r';
}

Token Lexer::read()
{
    this->err = error_ok();
    char c = this->pop();
    if (c == '\0')
    {
        return this->token_eof();
    }
    if (is_space(c))
    {
        while (is_space(this->peek()))
            this->pop();
        return this->empty();
    }
    TokenKind tk = single_op(c);
    if (tk != TokenKind::None)
    {
        return this->token(tk);
    }
    RET(this->op_dot(c));
    RET(this->op_equal(c));
    RET(this->op_less(c));
    RET(this->op_greater(c));
    RET(this->op_negate(c));
    RET(this->op_divide(c));
    RET(this->op_minus(c));
    RET(this->op_colon(c));
    RET(this->op_length(c));
    if (is_alphabetic(c))
    {
        return this->keyword_identifier(c);
    }
    if (is_digit(c))
    {
        return this->number(c);
    }
    if (c == '\'' || c == '"')
    {
        return this->short_string(c);
    }
    if (c == '[')
    {
        if (this->look_ahead())
        {
            return this->long_string();
        }
        else
        {
            return this->token(TokenKind::LeftBracket);
        }
    }
    return this->error(error_invalid_char(c));
}

Token Lexer::long_string()
{
    size_t level = 0;
    while (this->pop() != '[')
    {
        level++;
    }

    while (true)
    {
        char c = this->peek();
        if (c == '\0')
        {
            return this->error(error_missing_end_of_string(level));
        }
        this->pop();
        if (c == ']')
        {
            size_t lvl = level;
            while (true)
            {
                c = this->peek();
                if (c == '\0')
                    return this->error(error_missing_end_of_string(level));
                if (c == '=')
                {
                    this->pop();
                    lvl--;
                }
                else
                    break;
            }
            if (lvl == 0 && c == ']')
            {
                this->pop();
                break;
            }
        }
    }
    return this->token(TokenKind::Literal);
}

bool Lexer::look_ahead()
{
    size_t pos = this->pos;
    while (true)
    {
        char c = this->text[pos];
        if (c == '[')
        {
            return true;
        }
        else if (c != '=')
        {
            return false;
        }
        pos++;
    }
}

Token Lexer::short_string(char c)
{
    const char *escape_list = "abfnrtv\\\"\n'[]";
    bool escape = false;
    while (true)
    {
        char ch = this->pop();
        if ((ch == '\n' && !escape) || ch == '\0')
        {
            return this->error(error_missing_char(c));
        }
        if (escape)
        {
            if (ch == 'z')
            {
                while (this->peek() == '\n')
                {
                    this->pop();
                }
            }
            else if (ch == 'x')
            {
                for (int i = 0; i < 2; i++)
                {
                    if (!is_hex(this->peek()))
                        return this->error(error_invalid_escape());
                    this->pop();
                }
            }
            else if (is_digit(ch))
            {
                if (isdigit(this->peek()))
                {
                    char ch2 = this->pop();
                    if (is_digit(this->peek()))
                    {
                        char ch3 = this->pop();
                        ch -= '0';
                        ch2 -= '0';
                        ch3 -= '0';
                        int n = ch * 100 + ch2 * 10 + ch3;
                        if (n > 255)
                        {
                            return this->error(error_invalid_escape());
                        }
                    }
                }
            }
            else
            {
                bool exists = false;
                for (const char *ptr = escape_list; *ptr != '\0'; ptr++)
                {
                    if (ch == *ptr)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    return this->error(error_invalid_escape());
                }
            }
            escape = false;
        }
        else
        {
            if (ch == '\\')
            {
                escape = true;
            }
            if (ch == c)
            {
                break;
            }
        }
    }
    return this->token(TokenKind::Literal);
}

Token Lexer::hex()
{
    this->pop();
    if (!is_hex(this->peek()))
        return this->error(error_malformed_number());
    while (is_hex(this->peek()))
        this->pop();
    char c = this->peek();
    if (is_alphabetic(c) || c == '.')
        return this->error(error_malformed_number());
    return this->none();
}

Token Lexer::integer()
{
    while (is_digit(this->peek()))
        this->pop();
    char c = this->peek();
    if (is_alphabetic(c) && c != 'e')
        return this->error(error_malformed_number());
    return this->none();
}
Token Lexer::decimal()
{
    if (this->peek() == '.')
    {
        this->pop();
        while (is_digit(this->peek()))
            this->pop();
        char c = this->peek();
        if ((is_alphabetic(c) && c != 'e') || c == '.')
            return this->error(error_malformed_number());
    }
    return this->none();
}
Token Lexer::power()
{
    if (this->peek() == 'e')
    {
        this->pop();
        if (this->peek() == '-')
            this->pop();
        if (!is_digit(this->peek()))
            return this->error(error_malformed_number());
        while (is_digit(this->peek()))
            this->pop();
        char c = this->peek();
        if (is_alphabetic(c) || c == '.')
            return this->error(error_malformed_number());
    }
    return this->none();
}

Token Lexer::number(char c)
{
    if (c == '0' && this->peek() == 'x')
    {
        RET(this->hex());
    }
    else
    {
        RET(this->integer());
        RET(this->decimal());
        RET(this->power());
    }
    return this->token(TokenKind::Number);
}

Token Lexer::error(LError err)
{
    Token errtok = Token(nullptr, 0, this->prev_line, this->prev_offset, TokenKind::Error);
    this->err = err;
    this->err.line = errtok.line;
    this->err.offset = errtok.offset;
    return errtok;
}

Token Lexer::keyword_identifier(char c)
{
    while (is_alphanumeric(this->peek()))
    {
        this->pop();
    }
    TokenKind tk = this->keyword();
    if (tk != TokenKind::None)
    {
        return this->token(tk);
    }
    else
    {
        return this->token(TokenKind::Identifier);
    }
}

vector<Token> Lexer::drain()
{
    vector<Token> tokens;
    while (true)
    {
        Token t = this->next();
        tokens.push_back(t);
        if (t.kind == TokenKind::Eof)
            break;
    }
    return tokens;
}

Token Lexer::token(TokenKind kind)
{
    return Token((char *)this->text + this->prev_pos, this->pos - this->prev_pos, this->prev_line, this->prev_offset, kind);
}

char Lexer::ch(size_t offset)
{
    return this->text[this->prev_pos + offset];
}

void Lexer::skip_line()
{
    char c = this->peek();
    while (c != '\n' && c != '\0')
        c = this->pop();
}

LError Lexer::get_error()
{
    return this->err;
}

Token Lexer::skip_comment_block()
{
    size_t level = 0;
    while (this->peek() == '=')
    {
        this->pop();
        level++;
    }
    while (true)
    {
        char c = this->pop();
        if (c == ']')
        {
            size_t lvl = level;
            while (this->peek() == '=')
            {
                this->pop();
                lvl--;
            }
            char l = this->peek();
            if (l == '\0')
            {
                return this->error(error_missing_end_of_comment(level));
            }
            if (l == ']' && lvl == 0)
            {
                break;
            }
        }
        else if (c == '\0')
        {
            return this->error(error_missing_end_of_comment(level));
        }
    }
    this->pop();
    return this->empty();
}

Token Lexer::token_eof()
{
    return this->token(TokenKind::Eof);
}

Token Lexer::none()
{
    return this->token(TokenKind::None);
}

Token Lexer::op_equal(char c)
{
    if (c == '=')
    {
        if (this->peek() == '=')
        {
            pop();
            return this->token(TokenKind::EqualEqual);
        }
        else
            return this->token(TokenKind::Equal);
    }
    return this->none();
}
Token Lexer::op_length(char c)
{
    if (c == '#')
    {
        if (this->pos == 1 && this->peek() == '!')
        {
            pop();
            this->skip_line();
            return this->empty();
        }
        else
            return this->token(TokenKind::Length);
    }
    return this->none();
}
Token Lexer::op_colon(char c)
{
    if (c == ':')
    {
        if (this->peek() == ':')
        {
            pop();
            return this->token(TokenKind::ColonColon);
        }
        else
            return this->token(TokenKind::Colon);
    }
    return this->none();
}
Token Lexer::op_minus(char c)
{
    if (c == '-')
    {
        if (this->peek() == '-')
        {
            this->pop();
            if (this->peek() == '[')
            {
                this->pop();
                if (this->look_ahead())
                {
                    return this->skip_comment_block();
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
            return this->empty();
        }
        else
            return this->token(TokenKind::Minus);
    }
    return this->none();
}
Token Lexer::op_negate(char c)
{
    if (c == '~')
    {
        if (this->peek() == '=')
        {
            this->pop();
            return this->token(TokenKind::NotEqual);
        }
        else
            return this->token(TokenKind::Negate);
    }
    return this->none();
}
Token Lexer::op_dot(char c)
{
    if (c == '.')
    {
        if (this->peek() == '.')
        {
            this->pop();
            if (this->peek() == '.')
            {
                this->pop();
                return this->token(TokenKind::DotDotDot);
            }
            else
            {
                return this->token(TokenKind::DotDot);
            }
        }
        else if (is_digit(this->peek()))
        {
            return this->number(c);
        }
        else
            return this->token(TokenKind::Dot);
    }
    return this->none();
}
Token Lexer::op_divide(char c)
{
    if (c == '/')
    {
        if (this->peek() == '/')
        {
            pop();
            return this->token(TokenKind::FloorDivision);
        }
        else
            return this->token(TokenKind::FloatDivision);
    }
    return this->none();
}
Token Lexer::op_less(char c)
{
    if (c == '<')
    {
        if (this->peek() == '<')
        {
            pop();
            return this->token(TokenKind::LeftShift);
        }
        else if (this->peek() == '=')
        {
            pop();
            return this->token(TokenKind::LessEqual);
        }
        else
            return this->token(TokenKind::Less);
    }
    return this->none();
}
Token Lexer::op_greater(char c)
{
    if (c == '>')
    {
        if (this->peek() == '>')
        {
            pop();
            return this->token(TokenKind::RightShift);
        }
        else if (this->peek() == '=')
        {
            pop();
            return this->token(TokenKind::GreaterEqual);
        }
        else
            return this->token(TokenKind::Greater);
    }
    return this->none();
}