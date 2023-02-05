#ifndef AST_h
#define AST_h

#include <stddef.h>
#include <vector>
#include "lexer.h"

extern const char *node_names[34];

#define is_call(N) ((N)->get_kind() == NodeKind::Call || (N)->get_kind() == NodeKind::MethodCall)
#define is_vargs(N) ((N)->get_kind() == NodeKind::Primary && (N)->get_token().kind == TokenKind::DotDotDot)

#define scope(N) ((MetaScope *)N->getannot(MetaKind::MScope))
#define mdgoto(N) ((MetaGoto *)N->getannot(MetaKind::MGoto))
#define mdlabel(N) ((MetaLabel *)N->getannot(MetaKind::MLabel))
#define mem(N) ((MetaMemory *)N->getannot(MetaKind::MMemory))
#define is_meth(N) (N->get_kind() == NodeKind::MethodBody)

#define foreach_node(PARENT, CHILD) for ( \
    ast::Noderef CHILD = PARENT->begin(); \
    CHILD;                                \
    CHILD = CHILD->next())

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
        NEND = 34,     // ctrl
        TKN = 35,      // ctrl
    };

    class Node;
    typedef Node *Noderef;

    enum MetaKind
    {
        MDecl = 55,
        MMemory = 1,
        MLabel = 2,
        MScope = 3,
        MSelf = 4,
        MGoto = 5,
    };

    struct MetaData
    {
        MetaData *next;
        virtual MetaKind kind() = 0;
    };

    struct MetaSelf : public MetaData
    {
    };

    struct MetaDeclaration : public MetaData
    {
        Noderef decnode;
        bool is_upvalue;
    };

    struct MetaLabel : public MetaData
    {
        size_t stack_size;
        size_t upvalue_size;
        Noderef go_to;
        size_t address;
        bool is_compiled;
    };

    struct MetaGoto : public MetaData
    {
        size_t stack_size;
        size_t upvalue_size;
        size_t address;
        bool is_compiled;
        Noderef label;
        Noderef next;
    };

    struct MetaMemory : public MetaData
    {
        Noderef scope;
        size_t offset;
        bool is_upvalue;
        size_t upoffset;
    };

    struct MetaScope : public MetaData
    {
        Noderef func;
        Noderef parent;
        size_t stack_size;
        size_t upvalue_size;
        bool variadic;
        void *map;
        void *lmap;
        Noderef gotolist;
        fidx_t fidx;
    };

    class Node
    {

        friend class Ast;

    private:
        Token token;
        NodeKind kind;
        size_t count = 0;
        MetaData *meta = nullptr;

        Noderef parent = nullptr;
        Noderef right_sib = nullptr;
        Noderef left_sib = nullptr;
        Noderef left_child = nullptr;
        Noderef right_child = nullptr;

    public:
        Noderef next();
        NodeKind get_kind();
        Token get_token();
        Noderef child(size_t index);
        size_t child_count();
        Node(NodeKind kind, Token token = token_none());
        void annotate(MetaData *md);
        MetaData *getannot(MetaKind kind);
        int line();

        MetaGoto *metadata_goto();
        MetaLabel *metadata_label();
        MetaDeclaration *metadata_decl();
        MetaMemory *metadata_memory();
        MetaScope *metadata_scope();
        MetaSelf *metadata_self();

        static void
        sib_insert(Noderef l, Noderef r, Noderef s);
        void sib_insertl(Noderef node);
        void sib_insertr(Noderef node);
        void child_pushl(Noderef node);
        void child_pushr(Noderef node);
        Noderef begin();
        Noderef end();
        void pop();
        void replace(Noderef other);
    };

    class Ast
    {
    private:
        Noderef tree = nullptr;
        size_t *counter = nullptr;
        void destroy_node(Noderef node);
        void destroy();

    public:
        static Noderef make(NodeKind kind);
        static Noderef make(const vector<Noderef> &nodes, NodeKind kind);
        static Noderef make(vector<Noderef> &&nodes, NodeKind kind);
        static Noderef make(Token token, NodeKind kind);

        Ast &operator=(const Ast &other);
        Ast(const Ast &other);

        Ast &operator=(Ast &&other);
        Ast(Ast &&other);

        Noderef root();
        Ast(Noderef tree);
        ~Ast();
    };
}

#endif