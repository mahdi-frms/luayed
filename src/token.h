#ifndef TOKEN_h
#define TOKEN_h

#include "lyddef.h"

#define TOKEN_IS_PRIMARY(K) (K & 0x0100)
#define TOKEN_IS_POSTFIX(K) (K & 0x0200)
#define TOKEN_IS_BINARY(K) (K & 0x0400)
#define TOKEN_IS_PREFIX(K) (K & 0x0800)
#define TOKEN_IS_CONTROL(K) (K & 0x1000)
#define TOKEN_IS_STATEMENT(K) (K & 0x2000)

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
    Then = 0x0006,
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
    For = 0x200c,
    In = 0x200d,
    Function = 0x200e,
    Local = 0x200f,
    Return = 0x2010,

    Eof = 0x1000,

    None = 0x1001,  // control purpose (token not matched)
    Error = 0x1002, // returned as lexical error
    Empty = 0x1003, // control purpose (blank token)
};

struct Token
{
    size_t ptr;
    size_t len;
    size_t line;
    size_t offset;
    TokenKind kind;

    Token(size_t ptr, size_t len, size_t line, size_t offset, TokenKind kind);
    string text(const char *str) const;
};

Token token_none();

#endif