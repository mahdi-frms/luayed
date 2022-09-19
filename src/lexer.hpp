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
    Concat,
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

    Eof
};

struct Token
{
    string text;
    size_t line;
    size_t offset;
    TokenKind kind;

    Token(string text, size_t line, size_t offset, TokenKind kind);
};

class Lexer
{
private:
    string &text;
    vector<Token> tokens;
    size_t pos;

public:
    Lexer(string &text);
    Token next();
    vector<Token> lex();
};

#endif