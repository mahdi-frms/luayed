#ifndef AST_HPP
#define AST_HPP

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
        void stringify(int depth, string &buffer);

    public:
        Node(Gnode inner, NodeKind kind);
        string to_string();
    };

    class Ast
    {
    public:
        Ast(Noderef root);
        Noderef get_root();

    private:
        Noderef root;
    };
}

using namespace ast;

Noderef noderef(Node node);
Noderef make_binary(Noderef lexpr, Noderef rexpr, Token op);
Noderef make_unary(Noderef expr, Token op);
Noderef make_primary(Token token);

#endif