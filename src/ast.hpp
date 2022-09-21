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
    struct Property
    {
        Noderef table;
        Token field;
    };
    struct Index
    {
        Noderef table;
        Noderef idx;
    };
    struct Call
    {
        Noderef callee;
        Noderef arg;
    };
    struct Arglist
    {
        vector<Noderef> args;
    };
    struct Table
    {
        vector<Noderef> items;
    };
    struct Block
    {
        vector<Noderef> stmts;
    };
    struct CallStmt
    {
        Noderef call;
    };

    typedef std::variant<
        Primary,
        Binary,
        Unary,
        IdField,
        ExprField,
        Table,
        Arglist,
        Call,
        Property,
        Index,
        CallStmt,
        Block>

        Gnode;

    enum class NodeKind
    {
        Binary,
        Unary,
        Primary,
        IdField,
        ExprField,
        Table,
        Property,
        Index,
        Call,
        Arglist,
        CallStmt,
        Block
    };

    struct Node
    {
    private:
        Gnode inner;
        NodeKind kind;
        void stringify(int depth, string &buffer);

    public:
        NodeKind get_kind();
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
Noderef make_arglist(vector<Noderef> args);
Noderef make_call(Noderef callee, Noderef arg);
Noderef make_index(Noderef table, Noderef index);
Noderef make_property(Noderef table, Token field);
Noderef make_call_stmt(Noderef call);
Noderef make_block(vector<Noderef> args);

#endif