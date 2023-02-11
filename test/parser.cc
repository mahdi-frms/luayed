#include "test.h"
#include <ast.h>
#include <parser.h>
#include <stdarg.h>

#define T(K) NodeKind::TKN, K

using namespace luayed;

class MockLexer : public ILexer
{
private:
    vector<Token> tkns;

public:
    MockLexer(vector<Token> &tkns) : tkns(std::move(tkns))
    {
    }
    Token next()
    {
        Token last = this->tkns.front();
        this->tkns.erase(this->tkns.begin());
        return last;
    }
    Lerror get_error()
    {
        return error_ok();
    }
};

class AstMaker
{
private:
    vector<vector<Noderef>> nodes;
    vector<NodeKind> kinds;
    Noderef root;

public:
    void open(NodeKind kind)
    {
        nodes.push_back({});
        kinds.push_back(kind);
    }
    void add(Token t, NodeKind kind)
    {
        Noderef node = new Node(kind, t);

        if (nodes.size() == 0)
        {
            this->root = node;
        }
        else
        {
            nodes.back().push_back(node);
        }
    }
    void close()
    {
        Noderef node = new Node(this->kinds.back());
        for (size_t i = 0; i < nodes.back().size(); i++)
            node->child_pushr(nodes.back()[i]);
        nodes.pop_back();
        kinds.pop_back();

        if (nodes.size() == 0)
        {
            this->root = node;
        }
        else
        {
            nodes.back().push_back(node);
        }
    }
    Ast get_tree()
    {
        return Ast(root);
    }
};

bool is_token_node(NodeKind nk)
{
    return nk == Primary || nk == GotoStmt || nk == LabelStmt || nk == Operator || nk == Name;
}

bool cmp_node(Noderef n1, Noderef n2)
{
    if (n1->get_kind() != n2->get_kind())
        return false;
    if (is_token_node(n1->get_kind()))
        return n1->get_token().kind == n2->get_token().kind;
    if (n1->child_count() != n2->child_count())
        return false;
    for (size_t i = 0; i < n1->child_count(); i++)
        if (!cmp_node(n1->child(i), n2->child(i)))
            return false;
    return true;
}

Token tokenk(TokenKind k)
{
    return Token(0, 0, 0, 0, k);
}

void partest(bool exp, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vector<Token> tkns;
    AstMaker am;
    int depth = 0;

    do
    {
        NodeKind nk = (NodeKind)va_arg(args, int);
        if (is_token_node(nk))
        {
            va_arg(args, int); // TKN
            TokenKind tk = (TokenKind)va_arg(args, int);
            Token token = tokenk(tk);
            am.add(token, nk);
            tkns.push_back(token);
        }
        else if (nk == TKN)
        {
            TokenKind tk = (TokenKind)va_arg(args, int);
            Token token = tokenk(tk);
            tkns.push_back(token);
        }
        else if (nk == NEND)
        {
            depth--;
            am.close();
        }
        else
        {
            depth++;
            am.open(nk);
        }
    } while (depth);
    va_end(args);
    tkns.push_back(tokenk(TokenKind::Eof));

    MockLexer mlx = MockLexer(tkns);
    Parser parser = Parser(&mlx);
    Ast parser_ast = exp ? parser.parse_exp() : parser.parse();
    Ast maker_ast = am.get_tree();
    string mes = "parser : ";
    mes.append(message);
    test_assert(cmp_node(parser_ast.root(), maker_ast.root()), mes.c_str());
}

#define exptest(MES, ...) partest(true, MES, __VA_ARGS__)
#define chunktest(MES, ...) partest(false, MES, __VA_ARGS__)

void parser_tests()
{
    exptest("number",
            Primary,
            /**/ T(Number));
    exptest("string",
            Primary,
            /**/ T(Literal));
    exptest("boolean - true",
            Primary,
            /**/ T(True));
    exptest("boolean - false",
            Primary,
            /**/ T(False));
    exptest("nil",
            Primary,
            /**/ T(Nil));
    exptest("Identifier",
            Primary,
            /**/ T(Identifier));

    exptest("simple binary",
            Binary,
            /**/ Primary,
            /*  */ T(Identifier),
            /**/ Operator,
            /*  */ T(Plus),
            /**/ Primary,
            /*  */ T(Identifier),
            NEND);

    exptest("two binaries",
            Binary,
            /**/ Primary,
            /*  */ T(Number),
            /**/ Operator,
            /*  */ T(Plus),
            /**/ Binary,
            /*  */ Primary,
            /*      */ T(Number),
            /*  */ Operator,
            /*      */ T(Multiply),
            /*  */ Primary,
            /*      */ T(True),
            /**/ NEND,
            NEND);
}
