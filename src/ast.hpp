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
    struct IdField
    {
        Token field;
        Noderef value;
    };
    struct ExprField
    {
        Noderef field;
        Noderef value;
    };
    struct Table
    {
        vector<Noderef> items;
    };

    typedef std::variant<Primary, Binary, Unary, IdField, ExprField, Table> Gnode;

    enum class NodeKind
    {
        Binary,
        Unary,
        Primary,
        IdField,
        ExprField,
        Table
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
Noderef make_id_field(Token field, Noderef value);
Noderef make_expr_field(Noderef field, Noderef value);
Noderef make_table(vector<Noderef> items);

#endif