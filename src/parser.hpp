#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include <variant>
#include <memory>

template <typename T>
using shared = std::shared_ptr<T>;

namespace ast
{
    class Node;
    typedef shared<Node> Noderef;

    struct Binary
    {
        Noderef lexpr;
        Noderef rexpr;
        Token op;
    };
    struct Unary
    {
        Noderef expr;
        Token op;
    };
    struct Primary
    {
        Token token;
    };

    typedef std::variant<Primary, Binary, Unary> Gnode;

    enum class NodeKind
    {
        Binary,
        Unary,
        Primary,
    };

    struct Node
    {
    private:
        Gnode inner;
        NodeKind kind;

    public:
        Node(Gnode inner, NodeKind kind);
    };

    class Ast
    {
    public:
        Ast(Noderef root);

    private:
        Noderef root;
    };
}

using namespace ast;

class Parser
{
private:
    Lexer &lexer;
    Noderef expr();

public:
    Parser(Lexer &lexer);
    Ast parse();
};

#endif