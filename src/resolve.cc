#include "resolve.h"

// FIXME
#include "lstrep.h"

vector<Lerror> Resolver::analyze()
{
    this->current = nullptr;
    this->analyze_node(ast.root());
    return std::move(this->errors);
}
void Resolver::analyze_var_decl(Noderef node)
{
    node = node->child(0);
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::Identifier)
    {
        string name = tkn.text();
        this->curmap()[name] = node;
        MetaMemory *meta = new MetaMemory;
        meta->header.next = nullptr;
        meta->header.kind = MetaKind::MMemory;
        meta->offset = 0;
        meta->is_upvalue = 0;
        meta->upoffset = 0;
        meta->scope = this->current;
        this->curscope()->stack_size++;
        this->stack_ptr++;
        node->annotate(&meta->header);
    }
    else // DotDotDot
    {
        this->curscope()->variadic = true;
    }
}

MetaScope *Resolver::curscope()
{
    return scope(this->current);
}
Varmap &Resolver::curmap()
{
    return map(this->current);
}

void Resolver::reference(Noderef node, Noderef dec, bool func_past)
{
    MetaDeclaration *meta = new MetaDeclaration;
    meta->decnode = dec;
    meta->header.kind = MetaKind::MDecl;
    meta->header.next = nullptr;
    meta->is_upvalue = func_past;
    node->annotate(&meta->header);
    if (func_past)
    {
        MetaMemory *mm = mem(dec);
        MetaScope *sc = scope(mm->scope);
        if (!mm->is_upvalue)
        {
            this->hook_ptr++;
            sc->upvalue_size++;
        }
        mm->is_upvalue = true;
    }
}

void Resolver::self_ref(Noderef node)
{
    MetaSelf *meta = new MetaSelf;
    meta->header.kind = MetaKind::MSelf;
    meta->header.next = nullptr;
    node->annotate(&meta->header);
}

void Resolver::analyze_identifier(Noderef node)
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
            Lerror error = error_vargs_outside_function();
            error.line = t.line;
            error.offset = t.offset;
            this->errors.push_back(error);
        }
    }
}

void Resolver::analyze_etc(Noderef node)
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
        MetaScope *sc = new MetaScope;
        sc->header.next = nullptr;
        sc->header.kind = MetaKind::MScope;
        node->annotate(&sc->header);

        if (this->current)
            sc->func = is_fn ? node : this->curscope()->func;
        else
            sc->func = node;

        sc->map = new Varmap();
        sc->lmap = new Varmap();
        sc->variadic = node == this->ast.root();
        sc->parent = this->current;
        sc->stack_size = is_meth(node) ? 1 : 0;
        sc->fidx = 0;
        sc->upvalue_size = 0;
        sc->gotolist = nullptr;

        this->current = node;
    }

    foreach_node(node, ch)
    {
        this->analyze_node(ch);
    }
    if (new_scope)
    {
        this->link_labels();
        MetaScope *sc = this->curscope();
        this->stack_ptr -= sc->stack_size;
        this->hook_ptr -= sc->upvalue_size;
        delete (Varmap *)sc->map;
        delete (Varmap *)sc->lmap;
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

void Resolver::analyze_break(Noderef node)
{
    Noderef it = this->current;
    while (it && !is_loop(it->get_kind()))
        it = scope(it)->parent;
    if (!it)
    {
        Lerror err = error_breake_outside_loop();
        Token tkn = node->get_token();
        err.line = tkn.line;
        err.offset = tkn.offset;
        this->errors.push_back(err);
    }
    else
    {
        // add label
        Token virtual_token = Token(nullptr, 0, 0, 0, TokenKind::Identifier);
        Noderef virtual_label = ast::Ast::make(virtual_token, NodeKind::LabelStmt);
        it->sib_insertr(virtual_label);
        // annotate label
        MetaLabel *lmd = new MetaLabel;
        lmd->header.kind = MetaKind::MLabel;
        lmd->header.next = nullptr;
        lmd->is_compiled = false;
        lmd->go_to = nullptr;
        lmd->address = 0;
        virtual_label->annotate(&lmd->header);
        // add goto
        Noderef virtual_goto = ast::Ast::make(virtual_token, NodeKind::GotoStmt);
        node->replace(virtual_goto);
        // annotate goto
        MetaGoto *gtmd = new MetaGoto;
        gtmd->header.kind = MetaKind::MGoto;
        gtmd->header.next = nullptr;
        gtmd->address = 0;
        gtmd->is_compiled = false;
        gtmd->label = nullptr;
        gtmd->stack_size = this->stack_ptr;
        gtmd->upvalue_size = this->hook_ptr;
        virtual_goto->annotate(&gtmd->header);
        // link goto with label
        this->link(virtual_goto, virtual_label);
    }
}

void Resolver::link(Noderef go_to, Noderef label)
{
    MetaLabel *lmd = mdlabel(label);
    MetaGoto *gmd = mdgoto(go_to);
    gmd->label = label;
    gmd->next = lmd->go_to;
    lmd->go_to = go_to;
}

void Resolver::analyze_label(Noderef node)
{
    string name = node->get_token().text();
    Varmap *labels = (Varmap *)this->curscope()->lmap;
    if (labels->find(name) != labels->cend())
    {
        // todo: generate error
    }
    else
    {
        MetaLabel *lmd = (MetaLabel *)node->getannot(MetaKind::MLabel);
        if (!lmd)
        {
            (*labels)[name] = node;
            lmd = new MetaLabel;
            lmd->header.kind = MetaKind::MLabel;
            lmd->header.next = nullptr;
            lmd->is_compiled = false;
            lmd->go_to = nullptr;
            lmd->address = 0;
            node->annotate(&lmd->header);
        }
        lmd->stack_size = this->stack_ptr;
        lmd->upvalue_size = this->hook_ptr;
    }
}

void Resolver::analyze_goto(Noderef node)
{
    MetaScope *fnmd = scope(this->curscope()->func);
    Noderef gotolist = fnmd->gotolist;
    MetaGoto *gtmd = (MetaGoto *)node->getannot(MetaKind::MGoto);
    if (!gtmd)
    {
        gtmd = new MetaGoto;
        gtmd->header.kind = MetaKind::MGoto;
        gtmd->header.next = nullptr;
        gtmd->address = 0;
        gtmd->is_compiled = false;
        gtmd->label = nullptr;
        gtmd->next = gotolist;
        gtmd->stack_size = this->stack_ptr;
        gtmd->upvalue_size = this->hook_ptr;
        fnmd->gotolist = node;
        node->annotate(&gtmd->header);
    }
}

void Resolver::analyze_node(Noderef node)
{
    if (node->get_kind() == NodeKind::LabelStmt)
        this->analyze_label(node);
    else if (node->get_kind() == NodeKind::GotoStmt)
        this->analyze_goto(node);
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

void Resolver::analyze_declaration(Noderef node)
{
    if (node->child(0)->get_kind() == NodeKind::VarList) // var decl
    {
        if (node->child_count() > 1)
        {
            Noderef exps = node->child(1);
            this->analyze_node(exps);
        }
        Noderef vars = node->child(0);
        this->analyze_node(vars);
    }
    else // func decl
    {
        this->analyze_etc(node);
    }
}

void Resolver::link_labels()
{
    MetaScope *sc = this->curscope();
    Varmap *labels = (Varmap *)sc->lmap;
    Noderef gotolist = sc->gotolist;
    while (gotolist)
    {
        Noderef node = gotolist;
        MetaGoto *gmd = mdgoto(node);
        Noderef next = gmd->next;
        string name = node->get_token().text();
        auto lptr = labels->find(name);
        if (lptr != labels->cend())
        {
            this->link(node, lptr->second);
        }
        else
        {
            gmd->next = nullptr;
            // todo: generate error
        }
        gotolist = next;
    }
}

Resolver::Resolver(Ast ast) : ast(ast)
{
}
