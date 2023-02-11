#ifndef LEXER_h
#define LEXER_h

#include <string>
#include <vector>
#include "luadef.h"
#include "lerror.h"
#include "token.h"
#include "virtuals.h"

namespace luayed
{

    class Lexer : public ILexer
    {
    private:
        ISourceReader *reader;
        size_t pos;
        size_t offset;
        size_t line;

        size_t buffer_pos;
        size_t buffer_line;
        size_t buffer_offset;
        size_t buffer_count;

        size_t prev_offset;
        size_t prev_line;
        size_t prev_pos;

        Lerror err;

        char peek();
        char pop();
        void skip_line();
        Token keyword_identifier(char c);
        TokenKind keyword(const string &tkn);
        TokenKind kw(const char *str, const string &tkn, TokenKind kind);
        Token short_string(char c);
        Token long_string(size_t level);
        Token long_string(char c);
        Token number(char c);
        Token integer();
        Token decimal();
        Token power();
        Token hex();
        Token skip_comment_block(size_t level);
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
        Token error(Lerror err);
        Token none();
        Token empty();
        bool look_ahead();

    public:
        Lexer(ISourceReader *reader);
        Token next();
        Lerror get_error();
        vector<Token> drain();
    };

    Token token_none();
};

#endif