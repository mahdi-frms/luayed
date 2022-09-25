#ifndef AST_HPP
#define AST_HPP

#include <stddef.h>
#include <vector>
#include "lexer.hpp"

namespace ast
{
    enum NodeKind
    {
        Primary = 0, // token
        Binary = 1,
        Unary = 2,
        IdField = 3,
        ExprField = 4,
        Table = 5,
        Property = 6,
        Index = 7,
        Call = 8,
        Explist = 9,
        CallStmt = 10,
        AssignStmt = 11,
        LabelStmt = 12, // token
        BreakStmt = 13,
        GotoStmt = 14, // token
        WhileStmt = 15,
        RepeatStmt = 16,
        IfStmt = 17,
        NumericFor = 18,
        GenericFor = 19,
        ReturnStmt = 20,
        FunctionBody = 21,
        Declaration = 22,
        MethodCall = 23,
        Block = 24,
        Name = 25, // token
        NameList = 26,
        IfClause = 27,
        ElseClause = 28,
        ElseIfClause = 29,
        VarDecl = 30,
        VarList = 31,
        MethodBody = 32,
        Operator = 33, // token
    };

    class Node;
    typedef Node *Noderef;

    class Node
    {
    private:
        Token token;
        Noderef *children;
        size_t count;
        NodeKind kind;

        void stringify(int depth, std::string &buffer);

    public:
        NodeKind get_kind();
        Token get_token();
        Noderef *get_children();
        size_t child_count();
        Node(Token token, NodeKind kind);
        Node(Noderef *children, size_t count, NodeKind kind);
        Node(std::vector<Noderef> &children, NodeKind kind);
        Node(std::vector<Noderef> &children, Token token, NodeKind kind);
        std::string to_string();
    };

    class Ast
    {
    private:
        Noderef tree;
        void destroy_node(Noderef node);

    public:
        void destroy();
        Noderef root();
        Ast(Noderef tree);
    };
}

#endif