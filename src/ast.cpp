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

Noderef make_arglist(vector<Noderef> args)
{
    return noderef(Node(Arglist{.args = std::move(args)}, NodeKind::Arglist));
}

Noderef make_call(Noderef callee, Noderef arg)
{
    return noderef(Node(Call{.callee = callee, .arg = arg}, NodeKind::Call));
}

Noderef make_index(Noderef table, Noderef idx)
{
    return noderef(Node(Index{.table = table, .idx = idx}, NodeKind::Index));
}

Noderef make_property(Noderef table, Token field)
{
    return noderef(Node(Property{.table = table, .field = field}, NodeKind::Property));
}
Noderef make_call_stmt(Noderef call)
{
    return noderef(Node(CallStmt{.call = call}, NodeKind::CallStmt));
}
Noderef make_block(vector<Noderef> stmts)
{
    return noderef(Node(Block{.stmts = stmts}, NodeKind::Block));
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
    else if (kind == NodeKind::Unary)
    {
        Unary node = std::get<Unary>(this->inner);
        buffer += at_depth("Unary Node\n", depth);
        buffer += at_depth(node.op.text + "\n", depth + 3);
        node.expr->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Primary)
    {
        Primary node = std::get<Primary>(this->inner);
        buffer += at_depth("Primary Node\n", depth);
        buffer += at_depth(node.token.text + "\n", depth + 3);
    }
    else if (kind == NodeKind::Table)
    {
        Table node = std::get<Table>(this->inner);
        buffer += at_depth("Table\n", depth);
        for (int i = 0; i < node.items.size(); i++)
            node.items[i]->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::IdField)
    {
        IdField node = std::get<IdField>(this->inner);
        buffer += at_depth("Id Field\n", depth);
        buffer += at_depth(node.field.text + "\n", depth + 3);
        node.value->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::ExprField)
    {
        ExprField node = std::get<ExprField>(this->inner);
        buffer += at_depth("Expression Field\n", depth);
        node.field->stringify(depth + 3, buffer);
        node.value->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Arglist)
    {
        Arglist node = std::get<Arglist>(this->inner);
        buffer += at_depth("Arglist\n", depth);
        for (int i = 0; i < node.args.size(); i++)
            node.args[i]->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Call)
    {
        Call node = std::get<Call>(this->inner);
        buffer += at_depth("Call\n", depth);
        node.callee->stringify(depth + 3, buffer);
        node.arg->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Index)
    {
        Index node = std::get<Index>(this->inner);
        buffer += at_depth("Index\n", depth);
        node.table->stringify(depth + 3, buffer);
        node.idx->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Property)
    {
        Property node = std::get<Property>(this->inner);
        buffer += at_depth("Property\n", depth);
        node.table->stringify(depth + 3, buffer);
        buffer += at_depth(node.field.text + "\n", depth + 3);
    }
    else if (kind == NodeKind::Block)
    {
        Block node = std::get<Block>(this->inner);
        buffer += at_depth("Block\n", depth);
        for (int i = 0; i < node.stmts.size(); i++)
            node.stmts[i]->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::CallStmt)
    {
        CallStmt node = std::get<CallStmt>(this->inner);
        buffer += at_depth("Call Statement\n", depth);
        node.call->stringify(depth + 3, buffer);
    }
}
Noderef Ast::get_root()
{
    return this->root;
}

NodeKind Node::get_kind()
{
    return this->kind;
}
