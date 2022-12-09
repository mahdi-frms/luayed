#include "resolve.hpp"

#define scope(N) ((MetaScope *)N->getannot(MetaKind::MScope))
#define map(N) (*((Varmap *)(scope(N)->map)))
#define mem(N) ((MetaMemory *)N->getannot(MetaKind::MMemory))
#define halloc(T) ((T *)this->ast.get_heap().alloc(sizeof(T)))
#define is_meth(N) (N->get_kind() == NodeKind::MethodBody)

vector<SemanticError> SemanticAnalyzer::analyze()
{
    this->current = nullptr;
    this->analyze_node(ast.root());
    this->finalize();
    return std::move(this->errors);
}
void SemanticAnalyzer::analyze_var_decl(Noderef node)
{
    node = node->child(0);
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::Identifier)
    {
        string name = tkn.text();
        this->curmap()[name] = node;
        MetaMemory *meta = halloc(MetaMemory);
        meta->header.next = nullptr;
        meta->header.kind = MetaKind::MMemory;
        meta->offset = 0;
        meta->is_upvalue = 0;
        meta->upoffset = 0;
        meta->scope = this->current;
        this->curscope()->stack_size++;
        node->annotate(&meta->header);
    }
    else // DotDotDot
    {
        this->curscope()->variadic = true;
    }
}

MetaScope *SemanticAnalyzer::curscope()
{
    return scope(this->current);
}
Varmap &SemanticAnalyzer::curmap()
{
    return map(this->current);
}

void SemanticAnalyzer::reference(Noderef node, Noderef dec, bool func_past)
{
    MetaDeclaration *meta = halloc(MetaDeclaration);
    meta->decnode = dec;
    meta->header.kind = MetaKind::MDecl;
    meta->header.next = nullptr;
    meta->is_upvalue = func_past;
    node->annotate(&meta->header);
    if (func_past)
    {
        MetaMemory *mm = mem(dec);
        mm->is_upvalue = true;
        MetaScope *sc = scope(mm->scope);
        sc->upvalue_size++;
    }
}

void SemanticAnalyzer::self_ref(Noderef node)
{
    MetaSelf *meta = halloc(MetaSelf);
    meta->header.kind = MetaKind::MSelf;
    meta->header.next = nullptr;
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
        Noderef scptr = this->current;
        while (scptr)
        {
            func_past |= func;
            func = (scptr->get_kind() == NodeKind::FunctionBody);
            Varmap &vmap = map(scptr);
            if (vmap.find(t.text()) != vmap.cend())
            {
                dec = vmap[t.text()];
                break;
            }
            scptr = scope(scptr)->parent;
        }
        if (dec)
            this->reference(node, dec, func_past);
        else if (is_meth(this->curscope()->func) && t.text() == "self")
            this->self_ref(node);
    }
    else if (t.kind == TokenKind::DotDotDot)
    {
        if (!scope(this->curscope()->func)->variadic)
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
    bool is_fn = node->get_kind() == NodeKind::FunctionBody || is_meth(node);

    bool new_scope =
        node->get_kind() == NodeKind::Block ||
        node->get_kind() == NodeKind::GenericFor ||
        node->get_kind() == NodeKind::NumericFor ||
        node->get_kind() == NodeKind::WhileStmt ||
        node->get_kind() == NodeKind::RepeatStmt ||
        node->get_kind() == NodeKind::IfClause ||
        node->get_kind() == NodeKind::ElseClause ||
        node->get_kind() == NodeKind::ElseIfClause || is_fn;

    if (new_scope)
    {
        MetaScope *sc = halloc(MetaScope);
        sc->header.next = nullptr;
        sc->header.kind = MetaKind::MScope;
        node->annotate(&sc->header);

        if (this->current)
            sc->func = is_fn ? node : this->curscope()->func;
        else
            sc->func = node;

        sc->map = new Varmap();
        sc->variadic = false;
        sc->parent = this->current;
        sc->stack_size = is_meth(node) ? 1 : 0;
        sc->fidx = (is_fn || (!this->current)) ? this->fn_idx++ : 0;
        sc->upvalue_size = 0;

        this->current = node;
    }

    for (size_t i = 0; i < node->child_count(); i++)
    {
        this->analyze_node(node->child(i));
    }
    if (new_scope)
    {
        MetaScope *sc = this->curscope();
        delete (Varmap *)sc->map;
        this->current = sc->parent;
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
    Noderef it = this->current;
    while (!is_loop(it->get_kind()))
        it = scope(it)->parent;
    if (!it)
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

SemanticAnalyzer::SemanticAnalyzer(Ast ast) : ast(ast), fn_idx(1)
{
}
