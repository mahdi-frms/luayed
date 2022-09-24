#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

typedef std::string string;

template <typename T>
using vector = std::vector<T>;

enum class TokenKind
{
    Semicolon,
    Equal,
    Comma,
    ColonColon,
    Dot,
    Colon,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,

    Not,
    Negate,
    Length,

    Plus,
    Minus,
    Multiply,
    FloatDivision,
    FloorDivision,
    Modulo,
    Power,
    BinAnd,
    BinOr,
    RightShift,
    LeftShift,
    DotDot,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    EqualEqual,
    NotEqual,
    And,
    Or,

    Break,
    Goto,
    Do,
    End,
    While,
    Repeat,
    Until,
    If,
    Else,
    ElseIf,
    Then,
    For,
    In,
    Function,
    Local,
    Return,
    True,
    False,
    Nil,
    DotDotDot,

    Identifier,
    Number,
    Literal,

    Eof,

    None,  // returned as error
    Error, // returned as lexical error
    Empty, // control purpose
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
    string text();
};

enum class NumberScanPhase
{
    Integer,
    HEX,
    Decimal,
    Exponent,
    EarlyExponent,
};

class Lexer
{
private:
    string &text;
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
    Token number(char c, NumberScanPhase phase);
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
    Lexer(string &text);
    Token next();
    vector<Token> drain();
};

#endif