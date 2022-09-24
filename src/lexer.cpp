#include "lexer.hpp"
#include <cstring>

#define RET(T)                           \
    {                                    \
        auto tmp = T;                    \
        if (tmp.kind != TokenKind::None) \
            return tmp;                  \
    }                                    \
    0

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

char *error_missing(const char *symbols)
{
    const char *p1 = "missing symbol '";
    size_t l1 = strlen(p1);
    size_t ls = strlen(symbols);

    char *buf = (char *)malloc(l1 + ls + 1);
    strcpy(buf, p1);
    strcpy(buf + l1, symbols);
    buf[l1 + ls] = '\'';
    return buf;
}

char *error_missing_ch(char ch)
{
    const char *p1 = "missing symbol 'X'";
    size_t l1 = strlen(p1);

    char *buf = strdup(p1);
    buf[l1 - 2] = ch;
    return buf;
}

char *error_missing_ls(size_t level)
{
    const char *p1 = "missing symbol '[";
    size_t l1 = strlen(p1);

    char *buf = (char *)malloc(l1 + level + 2);
    strcpy(buf, p1);
    memset(buf + l1, level, '=');
    buf[l1 + level] = ']';
    buf[l1 + level + 1] = '\'';
    return buf;
}

char *error_invalid(char c)
{
    const char *p1 = "invalid character 'X'";
    size_t l1 = strlen(p1);

    char *buf = strdup(p1);
    buf[l1 - 2] = c;
    return buf;
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
TokenKind keyword(string &str)
{
    if (str.compare("and") == 0)
        return TokenKind::And;
    if (str.compare("or") == 0)
        return TokenKind::Or;
    if (str.compare("true") == 0)
        return TokenKind::True;
    if (str.compare("false") == 0)
        return TokenKind::False;
    if (str.compare("while") == 0)
        return TokenKind::While;
    if (str.compare("goto") == 0)
        return TokenKind::Goto;
    if (str.compare("repeat") == 0)
        return TokenKind::Repeat;
    if (str.compare("until") == 0)
        return TokenKind::Until;
    if (str.compare("for") == 0)
        return TokenKind::For;
    if (str.compare("local") == 0)
        return TokenKind::Local;
    if (str.compare("function") == 0)
        return TokenKind::Function;
    if (str.compare("break") == 0)
        return TokenKind::Break;
    if (str.compare("return") == 0)
        return TokenKind::Return;
    if (str.compare("nil") == 0)
        return TokenKind::Nil;
    if (str.compare("do") == 0)
        return TokenKind::Do;
    if (str.compare("end") == 0)
        return TokenKind::End;
    if (str.compare("if") == 0)
        return TokenKind::If;
    if (str.compare("else") == 0)
        return TokenKind::Else;
    if (str.compare("elseif") == 0)
        return TokenKind::ElseIf;
    if (str.compare("in") == 0)
        return TokenKind::In;
    if (str.compare("then") == 0)
        return TokenKind::Then;
    if (str.compare("not") == 0)
        return TokenKind::Not;
    return TokenKind::None;
}

string Token::text()
{
    string str = string(this->len, 0);
    memcpy((void *)str.c_str(), this->str, this->len);
    return str;
}

Token::~Token()
{
    if (this->kind == TokenKind::Error)
    {
        free((void *)this->str);
    }
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

Lexer::Lexer(string &text) : text(text)
{
    this->pos = 0;
    this->line = 0;
    this->offset = 0;
    this->prev_pos = 0;
    this->prev_line = 0;
    this->prev_offset = 0;
    this->tokens = vector<Token>();
}

Token::Token(const char *str, size_t len, size_t line, size_t offset, TokenKind kind)
{
    this->str = str;
    this->len = len;
    this->line = line;
    this->offset = offset;
    this->kind = kind;
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
        return this->number(c, NumberScanPhase::Integer);
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
    return this->error(error_invalid(c));
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
            return this->error(error_missing_ls(level));
        }
        this->pop();
        if (c == ']')
        {
            size_t lvl = level;
            while (true)
            {
                c = this->peek();
                if (c == '\0')
                    return this->error(error_missing_ls(level));
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
    const char *escape_list = "abfnrtxv\\\"\n'[]";
    const char *error_invescape = "invalid escape sequence";
    bool escape = false;
    while (true)
    {
        char ch = this->pop();
        if ((ch == '\n' && !escape) || ch == '\0')
        {
            return this->error(error_missing_ch(ch));
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
                            return this->error(strdup(error_invescape));
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
                    return this->error(strdup(error_invescape));
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

Token Lexer::number(char c, NumberScanPhase phase)
{
    const char *number_error = "malformed number";
    while (true)
    {
        c = this->peek();
        if (phase == NumberScanPhase::Integer)
        {
            if (c == '.')
            {
                phase = NumberScanPhase::Decimal;
            }
            else if (c == 'e')
            {
                phase = NumberScanPhase::EarlyExponent;
            }
            else if (c == 'x' && this->pos - this->prev_pos == 1 && this->text[this->prev_pos] == '0')
            {
                phase = NumberScanPhase::HEX;
            }
            else if (is_alphabetic(c))
            {
                return this->error(strdup(number_error));
            }
            else if (!is_digit(c))
                break;
            this->pop();
        }
        else if (phase == NumberScanPhase::HEX)
        {
            if (c == '.')
            {
                phase = NumberScanPhase::Decimal;
            }
            else if (c == 'e')
            {
                phase = NumberScanPhase::EarlyExponent;
            }
            else if (is_alphabetic(c) && !is_hex(c))
            {
                return this->error(strdup(number_error));
            }
            else if (!is_digit(c) && !is_hex(c))
                break;
            this->pop();
        }
        else if (phase == NumberScanPhase::Decimal)
        {
            if (c == 'e')
            {
                phase = NumberScanPhase::EarlyExponent;
            }
            else if (is_alphabetic(c))
            {
                return this->error(strdup(number_error));
            }
            else if (!is_digit(c))
                break;
            this->pop();
        }
        else if (phase == NumberScanPhase::EarlyExponent)
        {
            if (c == '-')
            {
                this->pop();
            }
            phase = NumberScanPhase::Exponent;
        }
        else // Exponent
        {
            if (is_alphabetic(c))
            {
                return this->error(strdup(number_error));
            }
            else if (!is_digit(c))
                break;
            this->pop();
        }
    }
    return this->token(TokenKind::Number);
}

Token Lexer::error(const char *message)
{
    Token err = Token(message, 0, this->prev_line, this->prev_offset, TokenKind::Error);
    this->skip_line();
    return err;
}

Token Lexer::keyword_identifier(char c)
{
    string id = string(1, c);
    while (is_alphanumeric(this->peek()))
    {
        id.push_back(this->pop());
    }
    TokenKind tk = keyword(id);
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
    while (true)
    {
        Token t = this->next();
        this->tokens.push_back(t);
        if (t.kind == TokenKind::Eof)
            break;
    }
    return std::move(this->tokens);
}

Token Lexer::token(TokenKind kind)
{
    return Token((char *)this->text.c_str() + this->prev_pos, this->pos - this->prev_pos, this->prev_line, this->prev_offset, kind);
}

void Lexer::skip_line()
{
    char c = this->peek();
    while (c != '\n' && c != '\0')
        c = this->pop();
}

Token Lexer::skip_comment_block()
{
    const char *error_miscom = "missing end of comment";
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
                return this->error(strdup(error_miscom));
            }
            if (l == ']' && lvl == 0)
            {
                break;
            }
        }
        else if (c == '\0')
        {
            return this->error(strdup(error_miscom));
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
            return this->number(c, NumberScanPhase::Decimal);
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