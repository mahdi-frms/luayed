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

Noderef make_explist(vector<Noderef> items)
{
    return noderef(Node(Explist{.items = std::move(items)}, NodeKind::Explist));
}

Noderef make_call(Noderef callee, Noderef arg)
{
    return noderef(Node(Call{.callee = callee, .arg = arg}, NodeKind::Call));
}
Noderef make_method_call(Noderef callee, Token name, Noderef arg)
{
    return noderef(Node(MethodCall{.callee = callee, .name = name, .arg = arg}, NodeKind::Call));
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

Noderef make_assign_stmt(Noderef varlist, Noderef explist)
{
    return noderef(Node(AssignStmt{.varlist = varlist, .explist = explist}, NodeKind::AssignStmt));
}

Noderef make_label_stmt(Token identifier)
{
    return noderef(Node(LabelStmt{.identifier = identifier}, NodeKind::LabelStmt));
}

Noderef make_break_stmt()
{
    return noderef(Node(BreakStmt{}, NodeKind::BreakStmt));
}

Noderef make_goto_stmt(Token identifier)
{
    return noderef(Node(GotoStmt{.identifier = identifier}, NodeKind::GotoStmt));
}
Noderef make_if_stmt(vector<Noderef> exprs, vector<Noderef> blocks)
{
    return noderef(Node(IfStmt{.exprs = exprs, .blocks = blocks}, NodeKind::IfStmt));
}
Noderef make_white_stmt(Noderef expr, Noderef block)
{
    return noderef(Node(WhileStmt{.expr = expr, .block = block}, NodeKind::WhileStmt));
}
Noderef make_repeat_stmt(Noderef expr, Noderef block)
{
    return noderef(Node(RepeatStmt{.expr = expr, .block = block}, NodeKind::RepeatStmt));
}
Noderef make_generic_for_stmt(vector<Token> namelist, Noderef explist, Noderef block)
{
    return noderef(Node(GenericFor{.namelist = namelist,
                                   .explist = explist,
                                   .block = block},
                        NodeKind::GenericFor));
}
Noderef make_numeric_for_stmt(Token identifier, Noderef expr_from, Noderef expr_to, Noderef expr_step, Noderef block)
{
    return noderef(Node(NumericFor{.identifier = identifier,
                                   .expr_from = expr_from,
                                   .expr_to = expr_to,
                                   .expr_step = expr_step,
                                   .block = block},
                        NodeKind::NumericFor));
}
Noderef make_return_stmt(Noderef expr)
{
    return noderef(Node(ReturnStmt{
                            .expr = expr,
                        },
                        NodeKind::ReturnStmt));
}

Noderef make_function_body(vector<Token> parlist, Noderef block)
{
    return noderef(Node(FunctionBody{.parlist = parlist, .block = block}, NodeKind::FunctionBody));
}

Noderef make_declaration(vector<Token> namelist, vector<Token> attriblist, Noderef explist)
{
    return noderef(Node(Declaration{.namelist = namelist,
                                    .attriblist = attriblist,
                                    .explist = explist},
                        NodeKind::Declaration));
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
    else if (kind == NodeKind::Explist)
    {
        Explist node = std::get<Explist>(this->inner);
        buffer += at_depth("Explist\n", depth);
        for (int i = 0; i < node.items.size(); i++)
            node.items[i]->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Call)
    {
        Call node = std::get<Call>(this->inner);
        buffer += at_depth("Call\n", depth);
        node.callee->stringify(depth + 3, buffer);
        node.arg->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::MethodCall)
    {
        MethodCall node = std::get<MethodCall>(this->inner);
        buffer += at_depth("Method Call\n", depth);
        node.callee->stringify(depth + 3, buffer);
        buffer += at_depth(node.name.text + "\n", depth + 3);
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
    else if (kind == NodeKind::AssignStmt)
    {
        AssignStmt node = std::get<AssignStmt>(this->inner);
        buffer += at_depth("Assign Statement\n", depth);
        node.varlist->stringify(depth + 3, buffer);
        node.explist->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::LabelStmt)
    {
        LabelStmt node = std::get<LabelStmt>(this->inner);
        buffer += at_depth("Label Statement\n", depth);
        buffer += at_depth(node.identifier.text + "\n", depth + 3);
    }
    else if (kind == NodeKind::BreakStmt)
    {
        BreakStmt node = std::get<BreakStmt>(this->inner);
        buffer += at_depth("Break Statement\n", depth);
    }
    else if (kind == NodeKind::GotoStmt)
    {
        GotoStmt node = std::get<GotoStmt>(this->inner);
        buffer += at_depth("Goto Statement\n", depth);
        buffer += at_depth(node.identifier.text + "\n", depth + 3);
    }
    else if (kind == NodeKind::ReturnStmt)
    {
        ReturnStmt node = std::get<ReturnStmt>(this->inner);
        buffer += at_depth("Return Statement\n", depth);
        if (node.expr)
            node.expr->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::WhileStmt)
    {
        WhileStmt node = std::get<WhileStmt>(this->inner);
        buffer += at_depth("While Statement\n", depth);
        node.expr->stringify(depth + 3, buffer);
        node.block->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::RepeatStmt)
    {
        RepeatStmt node = std::get<RepeatStmt>(this->inner);
        buffer += at_depth("Repeat Statement\n", depth);
        node.expr->stringify(depth + 3, buffer);
        node.block->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::IfStmt)
    {
        IfStmt node = std::get<IfStmt>(this->inner);
        buffer += at_depth("If Statement\n", depth);
        for (int i = 0; i < node.exprs.size(); i++)
        {
            if (node.exprs[i])
                node.exprs[i]->stringify(depth + 3, buffer);
            node.blocks[i]->stringify(depth + 3, buffer);
        }
    }
    else if (kind == NodeKind::GenericFor)
    {
        GenericFor node = std::get<GenericFor>(this->inner);
        buffer += at_depth("Generic For Statement\n", depth);
        for (int i = 0; i < node.namelist.size(); i++)
            buffer += at_depth(node.namelist[i].text + "\n", depth + 3);

        node.explist->stringify(depth + 3, buffer);
        node.block->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::NumericFor)
    {
        NumericFor node = std::get<NumericFor>(this->inner);
        buffer += at_depth("Numeric For Statement\n", depth);
        buffer += at_depth(node.identifier.text + "\n", depth + 3);
        node.expr_from->stringify(depth + 3, buffer);
        node.expr_to->stringify(depth + 3, buffer);
        if (node.expr_step)
        {
            node.expr_step->stringify(depth + 3, buffer);
        }
        node.block->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::FunctionBody)
    {
        FunctionBody node = std::get<FunctionBody>(this->inner);
        buffer += at_depth("Function Body\n", depth);
        for (int i = 0; i < node.parlist.size(); i++)
            buffer += at_depth(node.parlist[i].text + "\n", depth + 3);
        node.block->stringify(depth + 3, buffer);
    }
    else if (kind == NodeKind::Declaration)
    {
        Declaration node = std::get<Declaration>(this->inner);
        buffer += at_depth("Declaration\n", depth);
        for (int i = 0; i < node.namelist.size(); i++)
        {
            buffer += at_depth(node.namelist[i].text + "\n", depth + 3);
            if (node.attriblist[i].kind != TokenKind::None)
            {
                buffer += at_depth(node.attriblist[i].text + "\n", depth + 3);
            }
        }
        if (node.explist)
            node.explist->stringify(depth + 3, buffer);
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
