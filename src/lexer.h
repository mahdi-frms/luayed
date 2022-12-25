#ifndef LEXER_h
#define LEXER_h

#include <string>
#include <vector>
#include "luadef.h"
#include "lerror.h"
#include "token.h"

class ILexer
{
public:
    virtual LError get_error() = 0;
    virtual Token next() = 0;
};

class Lexer : ILexer
{
private:
    const char *text;
    size_t pos;
    size_t offset;
    size_t line;

    size_t prev_offset;
    size_t prev_line;
    size_t prev_pos;

    LError err;

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
    Token error(LError err);
    Token none();
    Token empty();
    bool look_ahead();

public:
    Lexer(const char *text);
    Token next();
    LError get_error();
    vector<Token> drain();
};

Token token_none();

#endif