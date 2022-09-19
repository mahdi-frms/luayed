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
};

struct Token
{
    string text;
    size_t line;
    size_t offset;
    TokenKind kind;

    Token(string text, size_t line, size_t offset, TokenKind kind);
};

enum class NumberScanPhase
{
    Integer,
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

    char peek();
    char read();
    void skip_line();
    Token keyword_identifier(char c);
    Token number(char c, NumberScanPhase phase);
    void skip_comment_block();
    void sync();
    Token pop();
    Token op_equal(char c);
    Token op_dot(char c);
    Token op_negate(char c);
    Token op_divide(char c);
    Token op_less(char c);
    Token op_greater(char c);
    Token op_minus(char c);
    Token token(string text, TokenKind kind);
    Token token_eof();
    Token error(string message);
    Token none();

public:
    Lexer(string &text);
    Token next();
    vector<Token> drain();
};

#endif