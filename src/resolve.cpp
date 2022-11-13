#include "resolve.hpp"

vector<SemanticError> SemanticAnalyzer::analyze()
{
    this->analyze_node(ast.root());
    this->finalize();
    return std::move(this->errors);
}

Scope &SemanticAnalyzer::curscope()
{
    return this->scopes.back();
}

void SemanticAnalyzer::analyze_var_decl(Noderef node)
{
    node = node->child(0);
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::Identifier)
    {
        string name = tkn.text();
        this->curscope().map[name] = node;
        MetaMemory *meta = (MetaMemory *)ast.get_heap().alloc(sizeof(MetaMemory));
        meta->is_stack = true;
        meta->header.next = nullptr;
        meta->header.kind = MetaKind::MMemory;
        meta->offset = this->curscope().stack_size++;
        node->annotate(&meta->header);
    }
    else // DotDotDot
    {
        this->curscope().variadic = true;
        this->curscope().stack_size = 256;
    }
}

void SemanticAnalyzer::analyze_identifier(Noderef node)
{
    Token t = node->get_token();
    if (t.kind == TokenKind::Identifier)
    {
        Noderef dec = nullptr;
        bool func = false;
        bool func_past = false;
        size_t idx = scopes.size() - 1;
        while (true)
        {
            func_past |= func;
            func = (this->scopes[idx].node->get_kind() == NodeKind::FunctionBody);
            if (this->scopes[idx].map.find(t.text()) != this->scopes[idx].map.cend())
            {
                dec = this->scopes[idx].map[t.text()];
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
            meta->header.kind = MetaKind::MDecl;
            meta->header.next = nullptr;
            node->annotate(&meta->header);
            if (func_past)
            {
                ((MetaMemory *)dec->getannot(MetaKind::MMemory))->is_stack = false;
            }
        }
    }
    else if (t.kind == TokenKind::DotDotDot)
    {
        bool valid = true;
        size_t idx = this->scopes.size() - 1;
        while (true)
        {
            Scope &sc = this->scopes[idx];
            if (sc.node->get_kind() == NodeKind::FunctionBody)
            {
                valid = sc.variadic;
            }
            if (idx == 0)
            {
                break;
            }
            idx--;
        }
        if (!valid)
        {
            SemanticError error;
            error.node = node;
            error.text = "cannot use '...' outside a vararg function";
            this->errors.push_back(error);
        }
    }
}

void SemanticAnalyzer::analyze_etc(Noderef node)
{
    bool new_scope =
        node->get_kind() == NodeKind::Block ||
        node->get_kind() == NodeKind::GenericFor ||
        node->get_kind() == NodeKind::NumericFor ||
        node->get_kind() == NodeKind::WhileStmt ||
        node->get_kind() == NodeKind::RepeatStmt ||
        node->get_kind() == NodeKind::IfClause ||
        node->get_kind() == NodeKind::ElseClause ||
        node->get_kind() == NodeKind::ElseIfClause ||
        node->get_kind() == NodeKind::FunctionBody;

    if (new_scope)
        this->scopes.push_back(Scope{
            .node = node,
            .map = Varmap(),
            .stack_size = this->scopes.size() ? this->curscope().stack_size : 0});

    for (size_t i = 0; i < node->child_count(); i++)
    {
        this->analyze_node(node->child(i));
    }
    if (new_scope)
    {
        MetaScope *md = (MetaScope *)this->ast.get_heap().alloc(sizeof(MetaScope));
        size_t prevsize = this->scopes.size() > 1 ? this->scopes[this->scopes.size() - 2].stack_size : 0;
        md->size = this->curscope().stack_size - prevsize;
        md->header.kind = MetaKind::MScope;
        md->header.next = nullptr;
        this->curscope().node->annotate(&md->header);
        this->scopes.pop_back();
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
    auto it = --this->scopes.cend();
    while (it != this->scopes.cbegin() && !is_loop(it->node->get_kind()))
        it--;
    if (it == this->scopes.cbegin())
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
        this->analyze_node(exps);
    }
    Noderef vars = node->child(0);
    this->analyze_node(vars);
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
            label->header.kind = MetaKind::MLabel;
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
