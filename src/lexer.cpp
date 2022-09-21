#include "lexer.hpp"

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
    char c = this->text[this->pos];
    if (c == '\n')
    {
        this->offset = 0;
        this->line++;
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
    this->prev_line = 0;
    this->prev_offset = 0;
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
    Token token = this->pop();
    this->sync();
    return token;
}

void Lexer::sync()
{
    this->prev_line = this->line;
    this->prev_offset = this->offset;
}

Token Lexer::pop()
{
    while (this->peek() != '\0')
    {
        char c = this->read();
        while (c == ' ' || c == '\n')
        {
            this->sync();
            c = this->read();
        }
        if (c == '\0')
        {
            break;
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
                return this->token(string("["), TokenKind::LeftBracket);
            }
        }
        this->sync();
    }
    return this->token_eof();
}

Token Lexer::long_string()
{
    string str = "[";
    size_t level = 0;
    while (this->read() != '[')
    {
        level++;
        str.push_back('=');
    }
    str.push_back('[');

    string error = string("missing symbol `]") + string(level, '=') + string("]`");

    while (true)
    {
        char c = this->read();
        if (c == '\0')
        {
            return this->error(error);
        }
        str.push_back(c);
        if (c == ']')
        {
            size_t lvl = level;
            while (true)
            {
                c = this->peek();
                if (c == '\0')
                    break;
                str.push_back(this->read());
                if (c == '=')
                    lvl--;
                else
                    break;
            }
            if (lvl == 0 && c == ']')
            {
                break;
            }
        }
    }

    return this->token(str, TokenKind::Literal);
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
    const char *escape_list = "abfnrtx\\\"'[]";
    bool escape = false;
    string str = string(1, c);
    while (true)
    {
        char ch = this->read();
        if (ch == '\n' || ch == '\0')
        {
            string message = string("missing symbol `");
            message.push_back(c);
            message.push_back('`');
            return this->error(message);
        }
        if (escape)
        {
            if (ch == 'z')
            {
                str.push_back(ch);
                while (this->peek() == '\n')
                {
                    str.push_back(this->read());
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
                    return this->error("invalid escape sequence");
                }
                str.push_back(ch);
            }
            escape = false;
        }
        else
        {
            if (ch == '\\')
            {
                escape = true;
            }
            str.push_back(ch);
            if (ch == c)
            {
                break;
            }
        }
    }
    return this->token(str, TokenKind::Literal);
}

Token Lexer::number(char c, NumberScanPhase phase)
{
    const char *number_error = "malformed number";
    string num = string(1, c);
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
            else if (is_alphabetic(c))
            {
                return this->error(number_error);
            }
            else if (!is_digit(c))
                break;
            num.push_back(this->read());
        }
        else if (phase == NumberScanPhase::Decimal)
        {
            if (c == 'e')
            {
                phase = NumberScanPhase::EarlyExponent;
            }
            else if (is_alphabetic(c))
            {
                return this->error(number_error);
            }
            else if (!is_digit(c))
                break;
            num.push_back(this->read());
        }
        else if (phase == NumberScanPhase::EarlyExponent)
        {
            if (c == '-')
            {
                num.push_back(this->read());
            }
            phase = NumberScanPhase::Exponent;
        }
        else // Exponent
        {
            if (is_alphabetic(c))
            {
                return this->error(number_error);
            }
            else if (!is_digit(c))
                break;
            num.push_back(this->read());
        }
    }
    return this->token(num, TokenKind::Number);
}

Token Lexer::error(string message)
{
    Token err = Token(message, this->line, this->offset, TokenKind::Error);
    this->skip_line();
    return err;
}

Token Lexer::keyword_identifier(char c)
{
    string id = string(1, c);
    while (is_alphanumeric(this->peek()))
    {
        id.push_back(this->read());
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
    return Token(text, this->prev_line, this->prev_offset, kind);
}

void Lexer::skip_line()
{
    char c = this->peek();
    while (c != '\n' && c != '\0')
        c = this->read();
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
    return this->token("", TokenKind::Eof);
}

Token Lexer::none()
{
    return this->token("", TokenKind::None);
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
                    this->skip_line();
                }
            }
            else
            {
                this->skip_line();
            }
            this->sync();
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