#include "ast.hpp"

const char *node_names[34] = {
    "Primary",
    "Binary",
    "Unary",
    "IdField",
    "ExprField",
    "Table",
    "Property",
    "Index",
    "Call",
    "Explist",
    "CallStmt",
    "AssignStmt",
    "LabelStmt",
    "BreakStmt",
    "GotoStmt",
    "WhileStmt",
    "RepeatStmt",
    "IfStmt",
    "NumericFor",
    "GenericFor",
    "ReturnStmt",
    "FunctionBody",
    "Declaration",
    "MethodCall",
    "Block",
    "Name",
    "NameList",
    "IfClause",
    "ElseClause",
    "ElseIfClause",
    "VarDecl",
    "VarList",
    "MethodBody",
    "Operator",
};

using namespace ast;

Node::Node(Token token, NodeKind kind) : token(token),
                                         children(nullptr), count(0), kind(kind)
{
}

Node::Node(std::vector<Noderef> &children, NodeKind kind) : token(token_none()), kind(kind)
{
    this->count = children.size();
    this->children = new Noderef[children.size()];
    for (size_t i = 0; i < children.size(); i++)
        this->children[i] = children[i];
}

Node::Node(std::vector<Noderef> &children, Token token, NodeKind kind) : token(token), kind(kind)
{
    this->count = children.size();
    this->children = new Noderef[children.size()];
    for (size_t i = 0; i < children.size(); i++)
        this->children[i] = children[i];
}

string at_depth(string text, int depth)
{
    string t = string(depth, '-');
    t += "> ";
    t += text;
    t += "\n";
    return t;
}

void Node::stringify(int depth, string &buffer)
{
    buffer += at_depth(node_names[this->kind], depth);
    if (this->token.kind != TokenKind::None)
    {
        buffer += at_depth(this->token.text(), depth + 3);
    }
    if (this->children)
    {
        for (size_t i = 0; i < this->count; i++)
            this->children[i]->stringify(depth + 3, buffer);
    }
}

Node::Node(Noderef *children, size_t count, NodeKind kind)
    : children(children), count(count), kind(kind), token(token_none())
{
}

string Node::to_string()
{
    string text = "";
    this->stringify(1, text);
    return text;
}

NodeKind Node::get_kind()
{
    return this->kind;
}

Token Node::get_token()
{
    return this->token;
}

Noderef *Node::get_children()
{
    return this->children;
}
size_t Node::child_count()
{
    return this->count;
}

void Ast::destroy_node(Noderef node)
{
    for (size_t i = 0; i < node->child_count(); i++)
    {
        this->destroy_node(node->get_children()[i]);
    }
    delete[] node->get_children();
    delete node;
}

void Ast::destroy()
{
    this->destroy_node(this->tree);
}

Ast::Ast(Noderef tree) : tree(tree)
{
}

Noderef Ast::root()
{
    return this->tree;
}
