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

public:
    Parser(Lexer &lexer);
    Ast parse();
};

#endif