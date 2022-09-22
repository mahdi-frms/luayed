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
    struct Explist
    {
        vector<Noderef> items;
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
    struct AssignStmt
    {
        Noderef varlist;
        Noderef explist;
    };
    struct LabelStmt
    {
        Token identifier;
    };
    struct BreakStmt
    {
    };
    struct GotoStmt
    {
        Token identifier;
    };
    struct WhileStmt
    {
        Noderef expr;
        Noderef block;
    };
    struct RepeatStmt
    {
        Noderef expr;
        Noderef block;
    };
    struct IfStmt
    {
        vector<Noderef> exprs;
        vector<Noderef> blocks;
    };
    struct NumericFor
    {
        Token identifier;
        Noderef expr_from;
        Noderef expr_to;
        Noderef expr_step;
        Noderef block;
    };
    struct GenericFor
    {
        vector<Token> namelist;
        Noderef explist;
        Noderef block;
    };
    struct ReturnStmt
    {
        Noderef expr;
    };
    struct FunctionBody
    {
        vector<Token> parlist;
        Noderef block;
    };
    struct Declaration
    {
        vector<Token> namelist;
        vector<Token> attriblist;
        Noderef explist;
    };
    struct MethodCall
    {
        Noderef callee;
        Token name;
        Noderef arg;
    };

    typedef std::variant<
        Primary,
        Binary,
        Unary,
        IdField,
        ExprField,
        Table,
        Explist,
        Call,
        Property,
        Index,
        CallStmt,
        AssignStmt,
        LabelStmt,
        BreakStmt,
        GotoStmt,
        WhileStmt,
        RepeatStmt,
        IfStmt,
        NumericFor,
        GenericFor,
        ReturnStmt,
        FunctionBody,
        Declaration,
        MethodCall,
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
        Explist,
        CallStmt,
        AssignStmt,
        LabelStmt,
        BreakStmt,
        GotoStmt,
        WhileStmt,
        RepeatStmt,
        IfStmt,
        NumericFor,
        GenericFor,
        ReturnStmt,
        FunctionBody,
        Declaration,
        MethodCall,
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

        template <typename T>
        T &as()
        {
            return std::get<T>(this->inner);
        }
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
Noderef make_explist(vector<Noderef> items);
Noderef make_call(Noderef callee, Noderef arg);
Noderef make_method_call(Noderef callee, Token name, Noderef arg);
Noderef make_index(Noderef table, Noderef index);
Noderef make_property(Noderef table, Token field);
Noderef make_call_stmt(Noderef call);
Noderef make_block(vector<Noderef> args);
Noderef make_assign_stmt(Noderef varlist, Noderef explist);
Noderef make_label_stmt(Token identifier);
Noderef make_break_stmt();
Noderef make_return_stmt(Noderef expr);
Noderef make_goto_stmt(Token identifier);
Noderef make_if_stmt(vector<Noderef> exprs, vector<Noderef> blocks);
Noderef make_white_stmt(Noderef expr, Noderef block);
Noderef make_repeat_stmt(Noderef expr, Noderef block);
Noderef make_generic_for_stmt(vector<Token> namelist, Noderef explist, Noderef block);
Noderef make_numeric_for_stmt(Token identifier, Noderef expr_from, Noderef expr_to, Noderef expr_step, Noderef block);
Noderef make_function_body(vector<Token> parlist, Noderef block);
Noderef make_declaration(vector<Token> namelist, vector<Token> attriblist, Noderef explist);

#endif