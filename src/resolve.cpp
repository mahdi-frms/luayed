#include "resolve.hpp"

vector<SemanticError> SemanticAnalyzer::analyze()
{

    maps.push_back(Varmap());
    node_kind.push_back(NodeKind::Block);
    this->analyze_node(ast.root());
    maps.pop_back();
    node_kind.pop_back();
    this->finalize();
    return std::move(this->errors);
}

void SemanticAnalyzer::analyze_var_decl(Noderef node)
{
    string name = node->child(0)->get_token().text();
    maps.back()[name] = node;
    MetaMemory *meta = (MetaMemory *)ast.get_heap().alloc(sizeof(MetaMemory));
    meta->is_stack = true;
    meta->header.next = nullptr;
    meta->header.kind = MetaKind::Memory;
    node->annotate(&meta->header);
}

void SemanticAnalyzer::analyze_identifier(Noderef node)
{
    Token t = node->get_token();
    if (t.kind == TokenKind::Identifier)
    {
        Noderef dec = nullptr;
        bool func = false;
        bool func_past = false;
        size_t idx = maps.size() - 1;
        while (true)
        {
            func_past |= func;
            func = (node_kind[idx] == NodeKind::FunctionBody);
            if (maps[idx].find(t.text()) != maps[idx].cend())
            {
                dec = maps[idx][t.text()];
                break;
            }
            if (idx == 0)
                break;
            idx--;
        }
        if (dec)
        {
            MetaDeclaration *meta = (MetaDeclaration *)ast.get_heap().alloc(sizeof(MetaDeclaration));
            meta->decnode = dec;
            meta->header.kind = MetaKind::Decl;
            meta->header.next = nullptr;
            node->annotate(&meta->header);
            if (func_past)
            {
                ((MetaMemory *)dec->getannot(MetaKind::Memory))->is_stack = false;
            }
        }
    }
}

void SemanticAnalyzer::analyze_etc(Noderef node)
{
    bool new_scope =
        node->get_kind() == NodeKind::Block ||
        node->get_kind() == NodeKind::GenericFor ||
        node->get_kind() == NodeKind::NumericFor ||
        node->get_kind() == NodeKind::FunctionBody;

    if (new_scope)
    {
        maps.push_back(Varmap());
        node_kind.push_back(node->get_kind());
    }
    for (size_t i = 0; i < node->child_count(); i++)
    {
        this->analyze_node(node->child(i));
    }
    if (new_scope)
    {
        size_t offset = 0;
        for (auto it = maps.back().cbegin(); it != maps.back().cend(); it++)
        {
            MetaMemory *meta = (MetaMemory *)it->second->getannot(MetaKind::Memory);
            if (meta && meta->is_stack)
            {
                meta->offset = offset++;
            }
        }

        maps.pop_back();
        node_kind.pop_back();
    }
}

bool is_loop(NodeKind kind)
{
    return kind == NodeKind::GenericFor ||
           kind == NodeKind::NumericFor ||
           kind == NodeKind::WhileStmt ||
           kind == NodeKind::RepeatStmt;
}

void SemanticAnalyzer::analyze_break(Noderef node)
{
    auto it = this->node_kind.cend();
    while (it != this->node_kind.cbegin() && !is_loop(*it))
        it--;
    if (it == this->node_kind.cbegin())
        this->errors.push_back(SemanticError{.node = node, .text = "break statement not in a loop"});
}

void SemanticAnalyzer::analyze_label(Noderef node)
{
    string name = node->child(0)->get_token().text();
    if (this->labels.find(name) != this->labels.cend())
    {
        this->errors.push_back(SemanticError{.node = node, .text = "redefined label"});
    }
    else
    {
        this->labels[name] = node;
    }
}

void SemanticAnalyzer::analyze_node(Noderef node)
{
    if (node->get_kind() == NodeKind::LabelStmt)
        this->analyze_label(node);
    else if (node->get_kind() == NodeKind::GotoStmt)
        gotolist.push_back(node);
    else if (node->get_kind() == NodeKind::VarDecl)
        this->analyze_var_decl(node);
    else if (node->get_kind() == NodeKind::Declaration)
        this->analyze_declaration(node);
    else if (node->get_kind() == NodeKind::Primary)
        this->analyze_identifier(node);
    else if (node->get_kind() == NodeKind::BreakStmt)
        this->analyze_break(node);
    else
        this->analyze_etc(node);
}

void SemanticAnalyzer::analyze_declaration(Noderef node)
{
    if (node->child_count() > 1)
    {
        Noderef exps = node->child(1);
        this->analyze_etc(exps);
    }
    Noderef vars = node->child(0);
    this->analyze_etc(vars);
}

void SemanticAnalyzer::finalize()
{
    while (gotolist.size())
    {
        Noderef node = gotolist.back();
        string name = node->child(0)->get_token().text();
        if (this->labels.find(name) != this->labels.cend())
        {
            MetaLabel *label = (MetaLabel *)this->ast.get_heap().alloc(sizeof(MetaLabel));
            label->header.next = nullptr;
            label->header.kind = MetaKind::Label;
            label->label_node = node;
            node->annotate(&label->header);
        }
        else
        {
            this->errors.push_back(SemanticError{.node = node, .text = string("label '") + name + string("' does not exist")});
        }
        gotolist.pop_back();
    }
    this->labels.clear();
}

SemanticAnalyzer::SemanticAnalyzer(Ast ast) : ast(ast)
{
}
