#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

typedef std::string string;

template <typename T>
using vector = std::vector<T>;

enum TokenKind
{
    // bin
    Plus = 0x0400,
    Multiply = 0x0401,
    FloatDivision = 0x0402,
    FloorDivision = 0x0403,
    Modulo = 0x0404,
    Power = 0x0405,
    BinAnd = 0x0406,
    BinOr = 0x0407,
    RightShift = 0x0408,
    LeftShift = 0x0409,
    DotDot = 0x040a,
    Less = 0x040b,
    LessEqual = 0x040c,
    Greater = 0x040d,
    GreaterEqual = 0x040e,
    EqualEqual = 0x040f,
    NotEqual = 0x0410,
    And = 0x0411,
    Or = 0x0412,
    // bin-pre
    Minus = 0x0c13,
    Negate = 0x0c14,
    // pre
    Not = 0x0800,
    Length = 0x0801,
    // post
    LeftBracket = 0x0200,
    Dot = 0x0201,
    Colon = 0x0202,
    LeftParen = 0x0203,
    LeftBrace = 0x0204,
    // primary-post
    Literal = 0x0300,
    // primary
    True = 0x0100,
    False = 0x0101,
    Nil = 0x0102,
    DotDotDot = 0x0103,
    Identifier = 0x0104,
    Number = 0x0105,
    // etc
    Equal = 0x0001,
    Comma = 0x0002,
    RightBracket = 0x0003,
    RightParen = 0x0004,
    RightBrace = 0x0005,
    // keyword
    ColonColon = 0x2000,
    Semicolon = 0x2001,
    Break = 0x2002,
    Goto = 0x2003,
    Do = 0x2004,
    End = 0x2005,
    While = 0x2006,
    Repeat = 0x2007,
    Until = 0x2008,
    If = 0x2009,
    Else = 0x200a,
    ElseIf = 0x200b,
    Then = 0x200c,
    For = 0x200d,
    In = 0x200e,
    Function = 0x200f,
    Local = 0x2010,
    Return = 0x2011,

    Eof = 0x1000,

    None = 0x1001,  // returned as error
    Error = 0x1002, // returned as lexical error
    Empty = 0x1003, // control purpose
};

string token_kind_stringify(TokenKind kind);

struct Token
{
    const char *str;
    size_t len;
    size_t line;
    size_t offset;
    TokenKind kind;

    Token(const char *str, size_t len, size_t line, size_t offset, TokenKind kind);
    ~Token();
    Token &operator=(const Token &other);
    Token(const Token &other);
    string text();
};

class ILexer
{
public:
    virtual Token next() = 0;
};

class Lexer : ILexer
{
private:
    const char *text;
    vector<Token> tokens;
    size_t pos;
    size_t offset;
    size_t line;

    size_t prev_offset;
    size_t prev_line;
    size_t prev_pos;

    char peek();
    char pop();
    char ch(size_t offset);
    void skip_line();
    Token keyword_identifier(char c);
    TokenKind keyword();
    TokenKind kw(const char *str, size_t idx, TokenKind kind);
    Token short_string(char c);
    Token long_string();
    Token long_string(char c);
    Token number(char c);
    Token integer();
    Token decimal();
    Token power();
    Token hex();
    Token skip_comment_block();
    void sync();
    Token read();
    Token op_equal(char c);
    Token op_colon(char c);
    Token op_dot(char c);
    Token op_negate(char c);
    Token op_divide(char c);
    Token op_less(char c);
    Token op_greater(char c);
    Token op_minus(char c);
    Token op_length(char c);
    Token token(TokenKind kind);
    Token token_eof();
    Token error(const char *message);
    Token none();
    Token empty();
    bool look_ahead();

public:
    Lexer(const char *text);
    Token next();
    vector<Token> drain();
};

Token token_none();

#endif