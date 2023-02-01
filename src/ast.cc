#include "ast.h"

using namespace ast;

Node::Node(NodeKind kind, Token token)
    : token(token), kind(kind)
{
}

void Node::sib_insertl(Noderef node)
{
    Node::sib_insert(this->left_sib, this, node);
}
void Node::sib_insertr(Noderef node)
{
    Node::sib_insert(this, this->right_sib, node);
}
void Node::sib_insert(Noderef l, Noderef r, Noderef s)
{
    if (l)
        l->right_sib = s;
    if (r)
        r->left_sib = s;
    s->left_sib = l;
    s->right_sib = r;
}

void Node::child_pushl(Noderef node)
{
    if (this->count)
    {
        this->left_child->sib_insertl(node);
    }
    else
    {
        node->left_sib = node->right_sib = nullptr;
        this->right_child = node;
    }
    this->left_child = node;
    node->parent = this;
    this->count++;
}
void Node::child_pushr(Noderef node)
{
    if (this->count)
    {
        this->right_child->sib_insertr(node);
    }
    else
    {
        node->left_sib = node->right_sib = nullptr;
        this->left_child = node;
    }
    this->right_child = node;
    node->parent = this;
    this->count++;
}
void Node::pop()
{
    Noderef l = this->left_sib;
    Noderef r = this->right_sib;
    if (l)
        l->right_sib = r;
    else
        this->parent->left_child = r;
    if (r)
        r->left_sib = l;
    else
        this->parent->right_child = l;
    this->parent->count--;

    this->parent = nullptr;
    this->right_sib = nullptr;
    this->left_sib = nullptr;
}

NodeKind Node::get_kind()
{
    return this->kind;
}

Token Node::get_token()
{
    return this->token;
}

size_t Node::child_count()
{
    return this->count;
}

void Ast::destroy()
{
    if (this->tree)
        this->destroy_node(this->tree);
}
void Ast::destroy_node(Noderef node)
{
    while (node)
    {
        while (node->count)
        {
            Noderef ch = node->left_child;
            ch->pop();
            node->sib_insertr(ch);
        }
        while (node->meta)
        {
            MetaNode *md = node->meta;
            node->meta = md->next;
            delete md;
        }
        Noderef next = node->right_sib;
        delete node;
        node = next;
    }
}

Ast::Ast(Noderef tree) : tree(tree), counter(new size_t(1))
{
}

Ast &Ast::operator=(const Ast &other)
{
    this->counter = other.counter;
    (*this->counter)++;
    this->tree = other.tree;
    return *this;
}
Ast::Ast(const Ast &other)
{
    this->counter = other.counter;
    (*this->counter)++;
    this->tree = other.tree;
}
Ast &Ast::operator=(Ast &&other)
{
    this->counter = other.counter;
    this->tree = other.tree;
    other.tree = nullptr;
    other.counter = nullptr;
    return *this;
}
Ast::Ast(Ast &&other)
{
    this->counter = other.counter;
    this->tree = other.tree;
    other.tree = nullptr;
    other.counter = nullptr;
}
Ast::~Ast()
{
    if (this->counter && --(*this->counter) == 0)
    {
        this->destroy();
        delete this->counter;
    }
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
    Noderef ch = this->left_child;
    while (index--)
        ch = ch->right_sib;
    return ch;
}

MetaNode *Node::getannot(MetaKind kind)
{
    MetaNode *tmp = this->meta;
    while (tmp != nullptr && tmp->kind != kind)
        tmp = tmp->next;
    return tmp;
}

Noderef Ast::make(NodeKind kind)
{
    return new Node(kind);
}
Noderef Ast::make(const vector<Noderef> &nodes, NodeKind kind)
{
    Noderef node = new Node(kind);
    for (size_t i = 0; i < nodes.size(); i++)
    {
        node->child_pushr(nodes[i]);
    }
    return node;
}
Noderef Ast::make(vector<Noderef> &&nodes, NodeKind kind)
{
    vector<Noderef> nodelist = nodes;
    return Ast::make(nodelist, kind);
}
Noderef Ast::make(Token token, NodeKind kind)
{
    return new Node(kind, token);
}
Noderef Ast::make(Noderef c1, NodeKind kind)
{
    return Ast::make(vector<Noderef>{c1}, kind);
}
Noderef Ast::make(Noderef c1, Noderef c2, NodeKind kind)
{
    return Ast::make({c1, c2}, kind);
}
Noderef Ast::make(Noderef c1, Noderef c2, Noderef c3, NodeKind kind)
{
    return Ast::make({c1, c2, c3}, kind);
}
Noderef Ast::make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, NodeKind kind)
{
    return Ast::make({c1, c2, c3, c4}, kind);
}
Noderef Ast::make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, Noderef c5, NodeKind kind)
{
    return Ast::make({c1, c2, c3, c4, c5}, kind);
}
int Node::line()
{
    if (this->token.kind != TokenKind::None)
        return this->token.line;
    for (size_t i = 0; i < this->child_count(); i++)
    {
        int line = this->child(i)->line();
        if (line > -1)
            return line;
    }
    return -1;
}
