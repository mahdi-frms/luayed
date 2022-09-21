#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast.hpp"
#include <variant>
#include <memory>

using namespace ast;

class Parser
{
private:
    Lexer &lexer;
    vector<Token> tokens;
    Noderef expr();
    Noderef expr_p(uint8_t);
    Noderef primary();
    Token pop();
    Token peek();
    Token ahead();
    Noderef table();
    Noderef id_field();
    Noderef expr_field();
    Noderef arglist();
    Token consume(TokenKind kind);

public:
    Parser(Lexer &lexer);
    Ast parse();
};

#endif