#include "parser.hpp"
#include <utility>
using namespace ast;

Noderef noderef(Node node)
{
    return std::make_shared<Node>(std::move(node));
}

Noderef make_binary(Noderef lexpr, Noderef rexpr, Token op)
{
    return noderef(Node(Binary{.lexpr = lexpr, .rexpr = rexpr, .op = op}, NodeKind::Binary));
}

Noderef make_unary(Noderef expr, Token op)
{
    return noderef(Node(Unary{.expr = expr, .op = op}, NodeKind::Unary));
}

Noderef make_primary(Token token)
{
    return noderef(Node(Primary{.token = token}, NodeKind::Primary));
}

Ast::Ast(Noderef root) : root(root)
{
}

Parser::Parser(Lexer &lexer) : lexer(lexer)
{
}

Node::Node(Gnode inner, NodeKind kind) : inner(inner), kind(kind)
{
}

Noderef Parser::expr()
{
    return make_primary(Token("", 0, 0, TokenKind::Eof));
}

Ast Parser::parse()
{
    return Ast(this->expr());
}