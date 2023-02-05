#include "ast.h"

using namespace ast;

Node::Node(NodeKind kind, Token token)
    : token(token), kind(kind)
{
}

void Node::sib_insertl(Noderef node)
{
    Node::sib_insert(this->left_sib, this, node);
    node->parent = this->parent;
    this->parent->count++;
}
void Node::sib_insertr(Noderef node)
{
    Node::sib_insert(this, this->right_sib, node);
    node->parent = this->parent;
    this->parent->count++;
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
        Node::sib_insert(nullptr, this->left_child, node);
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
        Node::sib_insert(this->right_child, nullptr, node);
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
            Node::sib_insert(node, node->right_sib, ch);
        }
        while (node->meta)
        {
            MetaData *md = node->meta;
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

void Node::annotate(MetaData *md)
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

MetaData *Node::getannot(MetaKind kind)
{
    MetaData *tmp = this->meta;
    while (tmp != nullptr && tmp->kind() != kind)
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
int Node::line()
{
    if (this->token.kind != TokenKind::None)
        return this->token.line;
    foreach_node(this, ch)
    {
        int line = ch->line();
        if (line > -1)
            return line;
    }
    return -1;
}
Noderef Node::next()
{
    return this->right_sib;
}
Noderef Node::begin()
{
    return this->left_child;
}
Noderef Node::end()
{
    return this->right_child;
}
void Node::replace(Noderef other)
{
    this->sib_insertr(other);
    this->pop();
    this->right_sib = other;
}

MetaGoto *Node::metadata_goto()
{
    return (MetaGoto *)this->getannot(MetaKind::MGoto);
}
MetaLabel *Node::metadata_label()
{
    return (MetaLabel *)this->getannot(MetaKind::MLabel);
}
MetaDeclaration *Node::metadata_decl()
{
    return (MetaDeclaration *)this->getannot(MetaKind::MDecl);
}
MetaMemory *Node::metadata_memory()
{
    return (MetaMemory *)this->getannot(MetaKind::MMemory);
}
MetaScope *Node::metadata_scope()
{
    return (MetaScope *)this->getannot(MetaKind::MScope);
}
MetaSelf *Node::metadata_self()
{
    return (MetaSelf *)this->getannot(MetaKind::MSelf);
}
MetaKind MetaGoto::kind()
{
    return MetaKind::MGoto;
}
MetaKind MetaLabel::kind()
{
    return MetaKind::MLabel;
}
MetaKind MetaDeclaration::kind()
{
    return MetaKind::MDecl;
}
MetaKind MetaMemory::kind()
{
    return MetaKind::MMemory;
}
MetaKind MetaSelf::kind()
{
    return MetaKind::MSelf;
}
MetaKind MetaScope::kind()
{
    return MetaKind::MScope;
}
MetaData::~MetaData()
{
}