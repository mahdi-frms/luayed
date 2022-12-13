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
    : token(token_none()), children(children), count(count), kind(kind)
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

void Ast::destroy()
{
    // FIXME : delete all nodes and metadata
}

Ast::Ast(Noderef tree) : tree(tree)
{
}

Noderef Ast::root()
{
    return this->tree;
}

void Node::annotate(MetaNode *md)
{
    md->next = this->meta;
    this->meta = md;
}

Noderef Node::child(size_t index)
{
    return this->children[index];
}

MetaNode *Node::getannot(MetaKind kind)
{
    MetaNode *tmp = this->meta;
    while (tmp != nullptr && tmp->kind != kind)
        tmp = tmp->next;
    return tmp;
}
