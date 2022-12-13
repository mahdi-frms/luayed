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
    ILexer *lexer;
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
    Noderef name_attrib();
    Noderef if_stmt();
    Noderef generic_for_stmt(Token identifier);
    Noderef numeric_for_stmt(Token identifier);
    Noderef arglist();
    Noderef varlist(Noderef var);
    Noderef explist();
    Noderef function_body(bool is_method);
    Token consume(TokenKind kind);
    Noderef make(NodeKind kind);
    Noderef make(vector<Noderef> &nodes, NodeKind kind);
    Noderef make(Token token, NodeKind kind);
    Noderef make(Noderef c1, NodeKind kind);
    Noderef make(Noderef c1, Noderef c2, NodeKind kind);
    Noderef make(Noderef c1, Noderef c2, Noderef c3, NodeKind kind);
    Noderef make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, NodeKind kind);
    Noderef make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, Noderef c5, NodeKind kind);

public:
    Parser(ILexer *lexer);
    Ast parse();
    Ast parse_exp();
};

#endif