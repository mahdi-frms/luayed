#include "resolve.h"

using namespace luayed;
using namespace luayed::ast;

vector<Lerror> Resolver::analyze()
{
    this->current = nullptr;
    this->analyze_node(ast.root());
    return std::move(this->errors);
}
size_t Resolver::new_var()
{
    this->curscope()->stack_size++;
    return this->stack_ptr++;
}
size_t Resolver::new_upvalue()
{
    return this->hook_ptr++;
}
void Resolver::analyze_var_decl(Noderef node)
{
    node = node->child(0);
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::Identifier)
    {
        string name = tkn.text(this->source);
        this->curmap()[name] = node;
        MetaMemory *meta = new MetaMemory;
        meta->scope = this->current;
        meta->offset = this->new_var();
        node->annotate(meta);
    }
    else // DotDotDot
    {
        this->curscope()->variadic = true;
    }
}

MetaGoto *Resolver::metadata_goto(Noderef node)
{
    MetaGoto *md = node->metadata_goto();
    if (!md)
    {
        md = new MetaGoto;
    }
    return md;
}
MetaLabel *Resolver::metadata_label(Noderef node)
{
    MetaLabel *md = node->metadata_label();
    if (!md)
    {
        md = new MetaLabel;
    }
    return md;
}
MetaDeclaration *Resolver::metadata_decl(Noderef node)
{
    MetaDeclaration *md = node->metadata_decl();
    if (!md)
    {
        md = new MetaDeclaration;
    }
    return md;
}
MetaMemory *Resolver::metadata_memory(Noderef node)
{
    MetaMemory *md = node->metadata_memory();
    if (!md)
    {
        md = new MetaMemory;
    }
    return md;
}
MetaScope *Resolver::metadata_scope(Noderef node)
{
    MetaScope *md = node->metadata_scope();
    if (!md)
    {
        md = new MetaScope;
    }
    return md;
}
MetaSelf *Resolver::metadata_self(Noderef node)
{
    MetaSelf *md = node->metadata_self();
    if (!md)
    {
        md = new MetaSelf;
    }
    return md;
}

MetaScope *Resolver::curscope()
{
    return this->current->metadata_scope();
}
Varmap &Resolver::curmap()
{
    return this->current->metadata_scope()->map;
}

void Resolver::reference(Noderef node, Noderef dec, bool func_past)
{
    MetaDeclaration *meta = new MetaDeclaration;
    meta->decnode = dec;
    meta->is_upvalue = func_past;
    node->annotate(meta);
    if (func_past)
    {
        MetaMemory *mm = dec->metadata_memory();
        MetaScope *sc = mm->scope->metadata_scope();
        if (!mm->is_upvalue)
        {
            mm->upoffset = this->new_upvalue();
            sc->upvalue_size++;
        }
        mm->is_upvalue = true;
    }
}

void Resolver::self_ref(Noderef node)
{
    MetaSelf *meta = new MetaSelf;
    node->annotate(meta);
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
            Varmap &vmap = scptr->metadata_scope()->map;
            if (vmap.find(t.text(this->source)) != vmap.cend())
            {
                dec = vmap[t.text(this->source)];
                break;
            }
            scptr = scptr->metadata_scope()->parent;
        }
        if (dec)
            this->reference(node, dec, func_past);
        else if (is_meth(this->curscope()->func) && t.text(this->source) == "self")
            this->self_ref(node);
    }
    else if (t.kind == TokenKind::DotDotDot)
    {
        if (!this->curscope()->func->metadata_scope()->variadic)
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

    size_t previous_stack_ptr;

    if (new_scope)
    {
        MetaScope *sc = new MetaScope;
        node->annotate(sc);

        if (this->current)
            sc->func = is_fn ? node : this->curscope()->func;
        else
            sc->func = node;

        sc->variadic = node == this->ast.root();
        sc->parent = this->current;
        sc->stack_size = is_meth(node) ? 1 : 0;
        this->current = node;

        if (is_fn)
        {
            previous_stack_ptr = this->stack_ptr;
            this->stack_ptr = is_meth(node) ? 1 : 0;
        }
    }

    if (node->get_kind() == NodeKind::NumericFor || node->get_kind() == NodeKind::GenericFor)
    {
        this->analyze_node(node->child(0));
        this->new_var();
        this->new_var();
        foreach_node_from(node, ch, 1)
        {
            this->analyze_node(ch);
        }
    }
    else
    {
        foreach_node(node, ch)
        {
            this->analyze_node(ch);
        }
    }

    if (new_scope)
    {
        this->link_labels();
        MetaScope *sc = this->curscope();
        this->stack_ptr -= sc->stack_size;
        this->hook_ptr -= sc->upvalue_size;
        this->current = sc->parent;
        if (is_fn)
        {
            this->stack_ptr = previous_stack_ptr;
        }
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
        it = it->metadata_scope()->parent;
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
        Token virtual_token = Token(0, 0, 0, 0, TokenKind::Identifier);
        Noderef virtual_label = ast::Ast::make(virtual_token, NodeKind::LabelStmt);
        it->sib_insertr(virtual_label);
        // annotate label
        virtual_label->annotate(new MetaLabel);
        // add goto
        Noderef virtual_goto = ast::Ast::make(virtual_token, NodeKind::GotoStmt);
        node->replace(virtual_goto);
        // annotate goto
        MetaGoto *gtmd = new MetaGoto;
        gtmd->stack_size = this->stack_ptr;
        gtmd->upvalue_size = this->hook_ptr;
        virtual_goto->annotate(gtmd);
        // link goto with label
        this->link(virtual_goto, virtual_label);
    }
}

void Resolver::link(Noderef go_to, Noderef label)
{
    MetaLabel *lmd = label->metadata_label();
    MetaGoto *gmd = go_to->metadata_goto();
    gmd->label = label;
    gmd->next = lmd->go_to;
    lmd->go_to = go_to;
}

void Resolver::analyze_label(Noderef node)
{
    string name = node->get_token().text(this->source);
    Varmap &labels = this->curscope()->lmap;
    auto prev_def = labels.find(name);
    if (prev_def != labels.cend())
    {
        Token label_token = prev_def->second->get_token();
        Lerror err = error_label_redefined(label_token.line, label_token.offset);
        err.line = node->get_token().line;
        err.offset = node->get_token().offset;
        this->errors.push_back(err);
    }
    else
    {
        MetaLabel *lmd = node->metadata_label();
        if (!lmd)
        {
            labels[name] = node;
            lmd = new MetaLabel;
            node->annotate(lmd);
        }
        lmd->stack_size = this->stack_ptr;
        lmd->upvalue_size = this->hook_ptr;
    }
}

void Resolver::analyze_goto(Noderef node)
{
    MetaScope *fnmd = this->curscope()->func->metadata_scope();
    Noderef gotolist = fnmd->gotolist;
    MetaGoto *gtmd = node->metadata_goto();
    if (!gtmd)
    {
        gtmd = new MetaGoto;
        gtmd->next = gotolist;
        gtmd->stack_size = this->stack_ptr;
        gtmd->upvalue_size = this->hook_ptr;
        fnmd->gotolist = node;
        node->annotate(gtmd);
    }
}

void Resolver::analyze_return(Noderef node)
{
    if (!node->child_count())
        return;
    Noderef explist = node->child(0);
    if (explist->child_count() == 1 && is_call(explist->child(0)))
    {
        explist->child(0)->annotate(new MetaTail());
    }
    this->analyze_node(explist);
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
    else if (node->get_kind() == NodeKind::ReturnStmt)
        this->analyze_return(node);
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
    Varmap &labels = sc->lmap;
    Noderef gotolist = sc->gotolist;
    while (gotolist)
    {
        Noderef node = gotolist;
        MetaGoto *gmd = node->metadata_goto();
        Noderef next = gmd->next;
        string name = node->get_token().text(this->source);
        auto lptr = labels.find(name);
        if (lptr != labels.cend())
        {
            this->link(node, lptr->second);
        }
        else
        {
            gmd->next = nullptr;
            Lerror err = error_label_undefined();
            err.line = node->get_token().line;
            err.offset = node->get_token().offset;
            this->errors.push_back(err);
        }
        gotolist = next;
    }
}

Resolver::Resolver(Ast ast, const char *source) : ast(ast), source(source)
{
}
