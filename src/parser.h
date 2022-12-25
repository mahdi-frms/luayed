#ifndef PARSER_h
#define PARSER_h

#include "ast.h"
#include <variant>
#include <memory>

using namespace ast;

enum class BlockEnd
{
    Eof,
    End,
    Else,
    Until,
};

class Parser
{
private:
    ILexer *lexer;
    Token current;
    LError err;

    Noderef expr();
    Noderef expr(Token t);
    Noderef fncall(Token op);
    Noderef expr_p(uint8_t pwr, Token tt);
    Noderef primary();
    Token pop();
    Token peek();
    Noderef table();
    Noderef id_field(Token t);
    Noderef expr_field();
    Noderef block(BlockEnd end);
    Noderef statement();
    Noderef while_stmt();
    Noderef repeat_stmt();
    Noderef vardecl();
    void error(LError err, Token token);
    Noderef name_attrib();
    Noderef if_stmt();
    Noderef generic_for_stmt(Token identifier);
    Noderef numeric_for_stmt(Token identifier);
    Noderef arglist();
    Noderef varlist(Noderef var);
    Noderef explist();
    Noderef function_body(bool is_method);
    Token consume(TokenKind kind);

public:
    Parser(ILexer *lexer);
    Ast parse();
    LError get_error();
    Ast parse_exp();
};

#endif