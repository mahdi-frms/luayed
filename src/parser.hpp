#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast.hpp"
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
    Lexer &lexer;
    Token current;
    Token ahead;

    Noderef expr();
    Noderef fncall(Token op);
    Noderef expr_p(uint8_t);
    Noderef primary();
    Token pop();
    Token peek();
    Token look_ahead();
    Noderef table();
    Noderef id_field();
    Noderef expr_field();
    Noderef block(BlockEnd end);
    Noderef statement();
    Noderef while_stmt();
    Noderef repeat_stmt();
    Noderef vardecl();
    void error(string message, Token token);
    Token name_attrib(Token *attrib);
    Noderef if_stmt();
    Noderef generic_for_stmt(Token identifier);
    Noderef numeric_for_stmt(Token identifier);
    Noderef arglist();
    Noderef varlist(Noderef var);
    Noderef explist();
    Noderef function_body();
    Token consume(TokenKind kind);

public:
    Parser(Lexer &lexer);
    Ast parse();
};

#endif