#include "compiler.h"

#define EXPECT_FREE 0xffff

int chex(char c)
{
    return c > 'a' ? (c - 'a' + 10) : (c - '0');
}
int cdec(char c)
{
    return (c <= '9' && c >= '0') ? (c - '0') : -1;
}

string scan_lua_string(Token t)
{
    string text = t.text();
    string str = "";
    bool escape = false;
    for (size_t i = 1; i < text.size() - 1; i++)
    {
        char c = text[i];
        if (escape)
        {
            escape = false;
            if (c == 'a')
                str.push_back('\a');
            else if (c == 'b')
                str.push_back('\b');
            else if (c == 'f')
                str.push_back('\f');
            else if (c == 'r')
                str.push_back('\r');
            else if (c == 't')
                str.push_back('\t');
            else if (c == 'n')
                str.push_back('\n');
            else if (c == '\n')
                str.push_back('\n');
            else if (c == '\v')
                str.push_back('\v');
            else if (c == '\\')
                str.push_back('\\');
            else if (c == '"')
                str.push_back('"');
            else if (c == '\'')
                str.push_back('\'');
            else if (c == 'x')
            {
                char c = chex(text[++i]) * 16;
                c += chex(text[++i]);
                str.push_back(c);
            }
            else
            {
                char v = 0;
                while (cdec(c) != -1)
                {
                    v *= 10;
                    v += c;
                    c = text[++i];
                }
                str.push_back(v);
            }
        }
        else
        {
            if (c == '\\')
                escape = true;
            else
                str.push_back(c);
        }
    }
    return str;
}

lnumber token_number(Token t)
{
    char *str = new char[t.len + 1];
    for (size_t i = 0; i < t.len; i++)
        str[i] = t.str[i];
    str[t.len] = '\0';
    lnumber num = atof(str);
    delete[] str;
    return num;
}

fidx_t Compiler::compile(Noderef root)
{
    MetaScope *fnscp = (MetaScope *)root->getannot(MetaKind::MScope);
    fnscp->fidx = this->gen->pushf();
    if (root->get_kind() == NodeKind::Block)
    {
        this->compile_node(root);
        this->gen->meta_parcount(0);
    }
    else
    {
        Noderef params = root->child(0);
        size_t parcount = 0;
        size_t upcount = 0;
        for (size_t i = 0; i < params->child_count(); i++)
        {
            Noderef par = params->child(i)->child(0);
            if (par->get_token().kind == TokenKind::DotDotDot)
                continue;
            parcount++;
            MetaMemory *md = (MetaMemory *)par->getannot(MetaKind::MMemory);
            md->offset = this->stack_offset++;
            if (md->is_upvalue)
            {
                md->upoffset = this->hooksize;
                this->hookpush();
                upcount++;
            }
        }
        this->compile_node(root->child(1));
        while (upcount--)
            this->emit(Instruction::IUPop);

        this->gen->meta_parcount(parcount);
    }
    this->gen->meta_hookmax(this->hookmax);
    this->emit(Opcode(Instruction::IRet, 0));
    this->gen->popf();
    return fnscp->fidx;
}

fidx_t Compiler::compile(Ast ast)
{
    return this->compile(ast.root());
}

lbyte tkn_binops[] = {
    IAdd,
    IMult,
    IFltDiv,
    IFlrDiv,
    IMod,
    IPow,
    IBAnd,
    IBOr,
    ISHR,
    ISHL,
    IConcat,
    ILt,
    ILe,
    IGt,
    IGe,
    IEq,
    INe};

lbyte Compiler::translate_token(TokenKind kind, bool bin)
{
    if (TOKEN_IS_BINARY(kind))
    {
        if (TOKEN_IS_PREFIX(kind))
        {
            return kind == TokenKind::Minus ? (bin ? Instruction::ISub : Instruction::INegate)
                                            : (bin ? Instruction::IBXor : Instruction::IBNot);
        }
        else
        {
            return tkn_binops[kind - 0x0400];
        }
    }
    else // prefix
    {
        return kind == TokenKind::Not ? Instruction::INot : Instruction::ILength;
    }
}

size_t Compiler::arglist_count(Noderef arglist)
{
    size_t chlen = arglist->child_count();
    if (!chlen)
        return 0;
    Noderef last = arglist->child(chlen - 1);
    if (last->get_kind() == NodeKind::MethodCall ||
        last->get_kind() == NodeKind::Call ||
        (last->get_kind() == NodeKind::Primary && last->get_token().kind == TokenKind::DotDotDot))
        chlen--;
    return chlen;
}
size_t Compiler::len()
{
    return this->gen->len();
}

void Compiler::compile_if(Noderef node)
{
    vector<size_t> jmps;
    size_t cjmp = 0;
    for (size_t i = 0; i < node->child_count(); i++)
    {
        Noderef cls = node->child(i);
        if (cls->get_kind() == NodeKind::ElseClause)
        {
            this->compile_block(cls->child(0));
        }
        else
        {
            this->compile_exp(cls->child(0));
            this->emit(Instruction::INot);
            cjmp = this->len();
            this->emit(Opcode(Instruction::ICjmp, 0));
            this->compile_block(cls->child(1));
            jmps.push_back(this->len());
            this->emit(Opcode(Instruction::IJmp, 0));
            size_t cjmp_idx = this->len();
            this->edit_jmp(cjmp, cjmp_idx);
        }
    }
    size_t jmp_idx = this->len();
    for (size_t i = 0; i < jmps.size(); i++)
    {
        this->edit_jmp(jmps[i], jmp_idx);
    }
}

void Compiler::edit_jmp(size_t opidx, size_t jmp_idx)
{
    this->seti(opidx + 1, jmp_idx % 256);
    this->seti(opidx + 2, jmp_idx >> 8);
}

void Compiler::compile_methcall(Noderef node, size_t expect)
{
    this->emit(Instruction::INil);
    this->compile_exp(node->child(0));
    this->emit(Opcode(Instruction::IBLocal, 1));
    size_t idx = this->const_string(node->child(1)->get_token().text().c_str());
    this->emit(Opcode(Instruction::IConst, idx));
    this->emit(Opcode(Instruction::ITGet));
    this->emit(Opcode(Instruction::IBLStore, 2));
    Noderef arglist = node->child(2);
    this->compile_explist(arglist, EXPECT_FREE);
    size_t argcount = this->arglist_count(arglist) + 1;
    if (expect == EXPECT_FREE)
        this->emit(Opcode(Instruction::ICall, argcount, 0));
    else
        this->emit(Opcode(Instruction::ICall, argcount, expect + 1));
}
void Compiler::compile_call(Noderef node, size_t expect)
{
    this->compile_exp(node->child(0));
    Noderef arglist = node->child(1);
    size_t argcount = this->arglist_count(arglist);
    this->compile_explist(arglist, EXPECT_FREE);
    if (expect == EXPECT_FREE)
        this->emit(Opcode(Instruction::ICall, argcount, 0));
    else
        this->emit(Opcode(Instruction::ICall, argcount, expect + 1));
}
void Compiler::compile_identifier(Noderef node)
{
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : node;
    MetaMemory *mm = (MetaMemory *)dec->getannot(MetaKind::MMemory);
    if (md)
    {
        if (md->is_upvalue)
        {
            MetaScope *sc = (MetaScope *)mm->scope->getannot(MetaKind::MScope);
            MetaScope *fnsc = (MetaScope *)sc->func->getannot(MetaKind::MScope);
            this->emit(Opcode(Instruction::IUpvalue, this->upval(fnsc->fidx, mm->offset, mm->upoffset)));
        }
        else
        {
            this->emit(Opcode(Instruction::ILocal, mm->offset));
        }
    }
    else if (node->getannot(MetaKind::MSelf))
    {
        this->emit(Opcode(Instruction::ILocal, 0));
    }
    else
    {
        size_t idx = this->const_string(node->get_token().text().c_str());
        this->emit(Opcode(Instruction::IConst, idx));
        this->emit(Instruction::IGGet);
    }
}

void Compiler::compile_primary(Noderef node, size_t expect)
{
    Token tkn = node->get_token();
    if (tkn.kind == TokenKind::True)
        this->emit(Instruction::ITrue);
    else if (tkn.kind == TokenKind::DotDotDot)
    {
        this->emit(Opcode(Instruction::IVargs, expect == EXPECT_FREE ? 0 : expect + 1));
    }
    else if (tkn.kind == TokenKind::False)
        this->emit(Instruction::IFalse);
    else if (tkn.kind == TokenKind::Nil)
        this->emit(Instruction::INil);
    else if (tkn.kind == TokenKind::Number)
    {
        size_t idx = this->const_number(token_number(tkn));
        this->emit(Opcode(Instruction::IConst, idx));
    }
    else if (tkn.kind == TokenKind::Literal)
    {
        size_t idx = this->const_string(scan_lua_string(tkn).c_str());
        this->emit(Opcode(Instruction::IConst, idx));
    }
    else if (tkn.kind == TokenKind::Identifier)
    {
        this->compile_identifier(node);
    }
}
void Compiler::compile_name(Noderef node)
{
    Token tkn = node->get_token();
    size_t idx = this->const_string(tkn.text().c_str());
    this->emit(Opcode(Instruction::IConst, idx));
}

void Compiler::compile_table(Noderef node)
{
    this->emit(Opcode(Instruction::ITNew));
    size_t list_len = 0;
    bool extra = false;
    if (!node->child_count())
        return;
    for (size_t i = 0; i < node->child_count(); i++)
    {
        Noderef ch = node->child(i);
        if (ch->get_kind() == NodeKind::IdField)
        {
            this->compile_name(ch->child(0));
            this->compile_exp(ch->child(1));
        }
        else if (ch->get_kind() == NodeKind::ExprField)
        {
            this->compile_exp(ch->child(0));
            this->compile_exp(ch->child(1));
        }
        else if (ch->get_kind() == NodeKind::Call || ch->get_kind() == NodeKind::MethodCall)
        {
            extra = true;
            this->compile_exp_e(ch, EXPECT_FREE);
        }
        else
        {
            list_len++;
            this->compile_exp(ch);
        }
        this->emit(Instruction::ITSet);
    }
    if (list_len || extra)
        this->emit(Opcode(Instruction::ITList, list_len));
}

void Compiler::compile_exp_e(Noderef node, size_t expect)
{
    bool is_vargs = node->get_kind() == NodeKind::Primary && node->get_token().kind == TokenKind::DotDotDot;
    bool is_fncall = node->get_kind() == NodeKind::Call || node->get_kind() == NodeKind::MethodCall;

    if (!expect && !is_fncall)
        return;
    if (node->get_kind() == NodeKind::Binary)
    {
        TokenKind tk = node->child(1)->get_token().kind;
        if (tk == TokenKind::And || tk == TokenKind::Or)
            this->compile_logic(node);
        else
        {
            this->compile_exp(node->child(0));
            this->compile_exp(node->child(2));
            this->emit(this->translate_token(node->child(1)->get_token().kind, true));
        }
    }
    else if (node->get_kind() == NodeKind::Unary)
    {
        this->compile_exp(node->child(1));
        this->emit(this->translate_token(node->child(0)->get_token().kind, false));
    }
    else if (node->get_kind() == NodeKind::Property)
    {
        this->compile_exp(node->child(0));
        size_t idx = this->const_string(node->child(1)->get_token().text().c_str());
        this->emit(Opcode(IConst, idx));
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::Index)
    {
        this->compile_exp(node->child(0));
        this->compile_exp(node->child(1));
        this->emit(ITGet);
    }
    else if (node->get_kind() == NodeKind::FunctionBody || node->get_kind() == NodeKind::MethodBody)
        this->compile_function(node);
    else if (node->get_kind() == NodeKind::Primary)
        this->compile_primary(node, expect);
    else if (node->get_kind() == NodeKind::Table)
        this->compile_table(node);
    else if (node->get_kind() == NodeKind::Call)
        this->compile_call(node, expect);
    else if (node->get_kind() == NodeKind::MethodCall)
        this->compile_methcall(node, expect);
    if (!is_fncall && !is_vargs && expect != EXPECT_FREE)
        while (--expect)
            this->emit(Opcode(Instruction::INil));
}

Compiler::Compiler(IGenerator *gen) : gen(gen)
{
}

void Compiler::compile_function(Noderef node)
{
    MetaScope *fnscp = (MetaScope *)node->getannot(MetaKind::MScope);
    Compiler compiler(this->gen);
    compiler.stack_offset = node->get_kind() == NodeKind::MethodBody ? 1 : 0;
    compiler.compile(node);
    compiler.emit(Opcode(Instruction::IFConst, fnscp->fidx));
}

void Compiler::compile_exp(Noderef node)
{
    this->compile_exp_e(node, 1);
}

MetaMemory *Compiler::varmem(Noderef lvalue)
{
    MetaDeclaration *md = (MetaDeclaration *)lvalue->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : lvalue;
    return (MetaMemory *)dec->getannot(MetaKind::MMemory);
}

void Compiler::compile_lvalue_primary(Noderef node)
{
    MetaDeclaration *md = (MetaDeclaration *)node->getannot(MetaKind::MDecl);
    Noderef dec = md ? md->decnode : node;
    MetaMemory *mm = (MetaMemory *)dec->getannot(MetaKind::MMemory);
    if (mm)
    {
        if (md && md->is_upvalue)
        {
            MetaScope *sc = (MetaScope *)mm->scope->getannot(MetaKind::MScope);
            MetaScope *fnsc = (MetaScope *)sc->func->getannot(MetaKind::MScope);
            this->ops_push(Opcode(Instruction::IUStore, this->upval(fnsc->fidx, mm->offset, mm->upoffset)));
        }
        else
        {
            this->ops_push(Opcode(Instruction::ILStore, mm->offset));
        }
    }
    else if (node->getannot(MetaKind::MSelf))
    {
        this->ops_push(Opcode(Instruction::ILStore, 0));
    }
    else
    {
        const char *str = node->get_token().text().c_str();
        size_t idx = this->const_string(str);
        this->emit(Opcode(Instruction::IConst, idx));
        this->ops_push(Instruction::IGSet);
        this->vstack.push_back(1);
    }
}

bool Compiler::compile_lvalue(Noderef node)
{
    if (node->get_kind() == NodeKind::Primary || node->get_kind() == NodeKind::Name)
    {
        this->compile_lvalue_primary(node);
        return false;
    }
    if (node->get_kind() == NodeKind::Property)
    {
        Noderef lexp = node->child(0);
        Noderef prop = node->child(1);
        this->compile_exp(lexp);
        const char *prop_str = prop->get_token().text().c_str();
        size_t idx = this->const_string(prop_str);
        this->emit(Opcode(Instruction::IConst, idx));
        this->ops_push(Opcode(Instruction::ITSet));
        this->vstack.push_back(1);
        this->vstack.push_back(1);
        return true;
    }
    if (node->get_kind() == NodeKind::Index)
    {
        Noderef lexp = node->child(0);
        Noderef iexp = node->child(1);
        this->compile_exp(lexp);
        this->compile_exp(iexp);
        this->ops_push(Opcode(Instruction::ITSet));
        this->vstack.push_back(1);
        this->vstack.push_back(1);
        return true;
    }
    return false; // never reaches
}

void Compiler::compile_explist(Noderef node, size_t vcount)
{
    if (vcount == EXPECT_FREE)
    {
        if (!node || node->child_count() == 0)
            return;
        for (size_t i = 0; i < node->child_count() - 1; i++)
        {
            this->compile_exp(node->child(i));
        }
        this->compile_exp_e(node->child(node->child_count() - 1), vcount);
    }
    else
    {
        if (!node || node->child_count() == 0)
        {
            while (vcount--)
                this->emit(Opcode(Instruction::INil));
        }
        else
        {
            for (size_t i = 0; i < node->child_count() - 1; i++)
            {
                this->compile_exp_e(node->child(i), vcount ? 1 : 0);
                if (vcount)
                    vcount--;
            }
            this->compile_exp_e(node->child(node->child_count() - 1), vcount);
        }
    }
}

size_t Compiler::compile_varlist(Noderef node)
{
    if (node->child_count() == 1)
    {
        return this->compile_lvalue(node->child(0)) ? 1 : 0;
    }
    else
    {
        size_t varlc = 0;
        for (size_t i = 0; i < node->child_count(); i++)
        {
            varlc += this->compile_lvalue(node->child(i)) ? 1 : 0;
            this->emit(Opcode(Instruction::INil));
            this->vstack.push_back(0);
        }
        return varlc;
    }
}

size_t Compiler::vstack_nearest_nil()
{
    size_t i = this->vstack.size() - 1;
    while (this->vstack[i])
        i--;
    this->vstack[i] = 1;
    return this->vstack.size() - 1 - i;
}

void Compiler::compile_generic_for_swap_pair(size_t back_offset1, size_t back_offset2)
{
    this->emit(Opcode(Instruction::IBLocal, back_offset1));
    this->emit(Opcode(Instruction::IBLocal, back_offset2 + 1));
    this->emit(Opcode(Instruction::IBLStore, back_offset1 + 1));
    this->emit(Opcode(Instruction::IBLStore, back_offset2));
}

void Compiler::compile_generic_for_swap(size_t varcount)
{
    // I
    // S
    // P
    //-----
    // P
    // S
    // I
    if (varcount == 1)
    {
        this->compile_generic_for_swap_pair(1, 3); // I <-> P
    }
    // I
    // S
    // P
    // nil
    //-----
    // P
    // nil
    // S
    // I
    else if (varcount == 2)
    {
        this->compile_generic_for_swap_pair(2, 4);
        this->compile_generic_for_swap_pair(2, 1);
        this->compile_generic_for_swap_pair(2, 3);
    }
    // I
    // S
    // P
    // nil
    // ...
    // nil
    //-----
    // P
    // nil
    // ...
    // nil
    // S
    // I
    else
    {
        this->compile_generic_for_swap_pair(1, varcount);
        this->compile_generic_for_swap_pair(2, varcount - 1);
        this->compile_generic_for_swap_pair(varcount, varcount - 2);
    }
}

void Compiler::compile_generic_for(Noderef node)
{
    Noderef varlist = node->child(0);
    Noderef lvalue = varlist->child(0)->child(0);
    Noderef arglist = node->child(1);
    size_t varcount = varlist->child_count();
    size_t upcount = 0;
    for (size_t i = 0; i < varcount; i++)
    {
        Noderef var = varlist->child(i)->child(0);
        this->varmem(var)->offset = this->stack_offset++;
        MetaMemory *mm = this->varmem(var);
        if (mm->is_upvalue)
        {
            upcount++;
        }
    }
    this->stack_offset += 2;
    //-- push args
    this->compile_explist(arglist, 3);
    for (size_t i = 0; i < varcount - 1; i++)
        this->emit(Instruction::INil);
    //-- push hooks
    for (size_t i = 0; i < upcount; i++)
    {
        this->hookpush();
        this->emit(Instruction::IUPush);
    }
    //-- swap args
    this->compile_generic_for_swap(varcount);
    //-- loop start
    size_t loop_beg = this->len();
    this->emit(Opcode(Instruction::IBLocal, 1));                           // iterator
    this->emit(Opcode(Instruction::IBLocal, 3));                           // state
    this->emit(Opcode(Instruction::ILocal, this->varmem(lvalue)->offset)); // prev
    this->emit(Opcode(Instruction::ICall, 2, varcount + 1));
    for (size_t i = 0; i < varcount; i++)
        this->emit(Opcode(Instruction::IBLStore, varcount + 2));
    //-- loop check
    this->emit(Opcode(Instruction::ILocal, this->varmem(lvalue)->offset));
    this->emit(Instruction::INil);
    this->emit(Instruction::IEq);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0x00)); // cjmp to loop end
    //-- block
    this->loop_start();
    this->compile_node(node->child(2));
    this->emit(Opcode(Instruction::IJmp, loop_beg));
    this->loop_end();
    size_t loop_end = this->len();
    //-- loop end
    this->emit(Opcode(Instruction::IPop, varcount + 2));
    this->stack_offset -= (varcount + 2);
    this->edit_jmp(cjmp, loop_end);
    for (size_t i = 0; i < upcount; i++)
    {
        this->hookpop();
        this->emit(Instruction::IUPop);
    }
}
void Compiler::seti(size_t idx, lbyte b)
{
    this->gen->seti(idx, b);
}
size_t Compiler::upval(fidx_t fidx, size_t offset, size_t hidx)
{
    return this->gen->upval(fidx, offset, hidx);
}

void Compiler::compile_numeric_for(Noderef node)
{
    Noderef lvalue = node->child(0)->child(0);
    MetaMemory *md = (MetaMemory *)lvalue->getannot(MetaKind::MMemory);
    md->offset = this->stack_offset++;
    this->stack_offset += 2;
    Noderef from = node->child(1);
    Noderef to = node->child(2);
    this->compile_exp(from);
    if (md->is_upvalue)
    {
        this->hookpush();
        this->emit(Instruction::IUPush);
    }
    this->compile_exp(to);
    size_t blk_idx = 3;
    if (node->child_count() == 5)
    {
        blk_idx++;
        this->compile_exp(node->child(3));
    }
    else
        this->emit(Opcode(Instruction::IConst, this->const_number(1)));
    size_t loop_start = this->len();
    this->loop_start();
    this->compile_block(node->child(blk_idx));
    this->emit(Opcode(Instruction::IBLocal, 3));
    this->emit(Opcode(Instruction::IBLocal, 2));
    this->emit(Instruction::IAdd);
    this->emit(Opcode(Instruction::IBLocal, 1));
    this->emit(Opcode(Instruction::IBLStore, 4));
    this->emit(Opcode(Instruction::IBLocal, 3));
    this->emit(Instruction::ILe);
    this->emit(Opcode(Instruction::ICjmp, loop_start));
    this->loop_end();
    if (md->is_upvalue)
    {
        this->hookpop();
        this->emit(Instruction::IUPop);
    }
    this->emit(Opcode(Instruction::IPop, 3));
    this->stack_offset -= 3;
}

void Compiler::compile_assignment(Noderef node)
{
    size_t vcount = node->child(0)->child_count();
    size_t varlc = this->compile_varlist(node->child(0));
    this->compile_explist(node->child(1), vcount);

    if (vcount > 1)
    {
        size_t v = vcount;
        while (v)
        {
            this->emit(Opcode(Instruction::IBLStore, v + this->vstack_nearest_nil()));
            v--;
        }
    }
    this->vstack.clear();
    this->ops_flush();
    if (varlc)
        this->emit(Opcode(Instruction::IPop, varlc));
}

void Compiler::compile_logic(Noderef node)
{
    this->compile_exp(node->child(0));
    this->emit(Opcode(Instruction::IBLocal, 1));
    if (node->child(1)->get_token().kind == TokenKind::And)
        this->emit(Instruction::INot);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->emit(Opcode(Instruction::IPop, 1));
    this->compile_exp(node->child(2));
    this->edit_jmp(cjmp, this->len());
}

void Compiler::compile_block(Noderef node)
{
    MetaScope *md = (MetaScope *)node->getannot(MetaKind::MScope);
    for (size_t i = 0; i < node->child_count(); i++)
        this->compile_node(node->child(i));
    for (size_t i = 0; i < md->upvalue_size; i++)
    {
        this->hookpop();
        this->emit(Instruction::IUPop);
    }
    if (md->stack_size)
    {
        this->emit(Opcode(Instruction::IPop, md->stack_size));
        this->stack_offset -= md->stack_size;
    }
}

void Compiler::compile_decl(Noderef node)
{
    Noderef varlist = node->child(0);
    size_t upcount = 0;
    for (size_t i = 0; i < varlist->child_count(); i++)
    {
        Noderef var = varlist->child(i)->child(0);
        MetaMemory *mm = (MetaMemory *)var->getannot(MetaKind::MMemory);
        mm->offset = this->stack_offset++;
        if (mm->is_upvalue)
        {
            mm->upoffset = this->hooksize;
            this->hookpush();
            upcount++;
        }
    }
    if (node->child_count() == 2)
        this->compile_explist(node->child(1), varlist->child_count());
    else
        for (size_t i = 0; i < varlist->child_count(); i++)
            this->emit(Opcode(Instruction::INil));
    while (upcount--)
    {
        this->emit(Instruction::IUPush);
    }
}

void Compiler::compile_ret(Noderef node)
{
    if (node->child_count())
    {
        Noderef vals = node->child(0);
        this->compile_explist(vals, EXPECT_FREE);
        this->emit(Opcode(Instruction::IRet, this->arglist_count(vals)));
    }
    else
    {
        this->emit(Opcode(Instruction::IRet, 0));
    }
}

void Compiler::compile_while(Noderef node)
{
    size_t jmp_idx = this->len();
    this->compile_exp(node->child(0));
    this->emit(Instruction::INot);
    size_t cjmp = this->len();
    this->emit(Opcode(Instruction::ICjmp, 0));
    this->loop_start();
    this->compile_block(node->child(1));
    this->emit(Opcode(Instruction::IJmp, jmp_idx));
    this->loop_end();
    this->edit_jmp(cjmp, this->len());
}

void Compiler::compile_repeat(Noderef node)
{
    size_t cjmp_idx = this->len();
    this->loop_start();
    this->compile_block(node->child(0));
    this->compile_exp(node->child(1));
    this->emit(Instruction::INot);
    this->emit(Opcode(Instruction::ICjmp, cjmp_idx));
    this->loop_end();
}

void Compiler::compile_break()
{
    this->breaks.push_back(this->len());
    this->emit(Opcode(Instruction::IJmp, 0));
}

void Compiler::compile_node(Noderef node)
{
    if (node->get_kind() == NodeKind::AssignStmt)
        this->compile_assignment(node);
    else if (node->get_kind() == NodeKind::Block)
        this->compile_block(node);
    else if (node->get_kind() == NodeKind::Declaration)
        this->compile_decl(node);
    else if (node->get_kind() == NodeKind::ReturnStmt)
        this->compile_ret(node);
    else if (node->get_kind() == NodeKind::CallStmt)
        this->compile_exp_e(node->child(0), 0);
    else if (node->get_kind() == NodeKind::IfStmt)
        this->compile_if(node);
    else if (node->get_kind() == NodeKind::WhileStmt)
        this->compile_while(node);
    else if (node->get_kind() == NodeKind::RepeatStmt)
        this->compile_repeat(node);
    else if (node->get_kind() == NodeKind::NumericFor)
        this->compile_numeric_for(node);
    else if (node->get_kind() == NodeKind::GenericFor)
        this->compile_generic_for(node);
    else if (node->get_kind() == NodeKind::BreakStmt)
        this->compile_break();
    else
        exit(4);
}

size_t Compiler::const_number(lnumber n)
{
    return this->gen->const_number(n);
}

size_t Compiler::const_string(const char *s)
{
    return this->gen->const_string(s);
}

void Compiler::emit(Opcode op)
{
    this->gen->emit(op);
}

void Compiler::ops_flush()
{
    while (this->ops.size())
    {
        this->emit(this->ops.back());
        this->ops.pop_back();
    }
}

void Compiler::ops_push(Opcode op)
{
    this->ops.push_back(op);
}

void Compiler::loop_start()
{
    this->breaks.push_back(0);
}
void Compiler::loop_end()
{
    size_t idx = this->len();
    while (this->breaks.back())
    {
        size_t brk = this->breaks.back();
        this->edit_jmp(brk, idx);
        this->breaks.pop_back();
    }
    this->breaks.pop_back();
}

void Compiler::hookpush()
{
    this->hooksize++;
    if (this->hookmax < this->hooksize)
        this->hookmax = this->hooksize;
}
void Compiler::hookpop()
{
    this->hooksize--;
}