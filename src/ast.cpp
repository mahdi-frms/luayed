#include "ast.hpp"

using namespace ast;

Noderef noderef(Node node)
{
    return std::make_shared<Node>(std::move(node));
}

Noderef make_binary(Noderef lexpr, Noderef rexpr, Token op)
{
    return noderef(Node(Binary{.lexpr = lexpr, .rexpr = rexpr, .op = op}, NodeKind::Binary));
}

Noderef make_unary(Noderef expr, Token op)
{
    return noderef(Node(Unary{.expr = expr, .op = op}, NodeKind::Unary));
}

Noderef make_primary(Token token)
{
    return noderef(Node(Primary{.token = token}, NodeKind::Primary));
}

Ast::Ast(Noderef root) : root(root)
{
}

string at_depth(string text, int depth)
{
    string t = string(depth, '-');
    t += "> ";
    t += text;
    return t;
}

string Node::to_string()
{
    string text = "";
    this->stringify(1, text);
    return text;
}

void Node::stringify(int depth, string &buffer)
{
    string text = "";
    NodeKind kind = this->kind;
    if (kind == NodeKind::Binary)
    {
        Binary node = std::get<Binary>(this->inner);
        buffer += at_depth("Binary Node\n", depth);
        node.lexpr->stringify(depth + 3, buffer);
        buffer += at_depth(node.op.text + "\n", depth + 3);
        node.rexpr->stringify(depth + 3, buffer);
    }
    if (kind == NodeKind::Unary)
    {
        Unary node = std::get<Unary>(this->inner);
        buffer += at_depth("Unary Node\n", depth);
        buffer += at_depth(node.op.text + "\n", depth + 3);
        node.expr->stringify(depth + 3, buffer);
    }
    if (kind == NodeKind::Primary)
    {
        Primary node = std::get<Primary>(this->inner);
        buffer += at_depth("Primary Node\n", depth);
        buffer += at_depth(node.token.text + "\n", depth + 3);
    }
}
Noderef Ast::get_root()
{
    return this->root;
}
