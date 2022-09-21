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

Noderef make_id_field(Token field, Noderef value)
{
    return noderef(Node(IdField{.field = field, .value = value}, NodeKind::IdField));
}

Noderef make_expr_field(Noderef field, Noderef value)
{
    return noderef(Node(ExprField{.field = field, .value = value}, NodeKind::ExprField));
}

Noderef make_table(vector<Noderef> items)
{
    return noderef(Node(Table{.items = std::move(items)}, NodeKind::Table));
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
    if (kind == NodeKind::Table)
    {
        Table node = std::get<Table>(this->inner);
        buffer += at_depth("Table\n", depth);
        for (int i = 0; i < node.items.size(); i++)
            node.items[i]->stringify(depth + 3, buffer);
    }
    if (kind == NodeKind::IdField)
    {
        IdField node = std::get<IdField>(this->inner);
        buffer += at_depth("Id Field\n", depth);
        buffer += at_depth(node.field.text + "\n", depth + 3);
        node.value->stringify(depth + 3, buffer);
    }
    if (kind == NodeKind::ExprField)
    {
        ExprField node = std::get<ExprField>(this->inner);
        buffer += at_depth("Expression Field\n", depth);
        node.field->stringify(depth + 3, buffer);
        node.value->stringify(depth + 3, buffer);
    }
}
Noderef Ast::get_root()
{
    return this->root;
}
