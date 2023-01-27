#ifndef COMPILER_h
#define COMPILER_h

#include "ast.h"
#include "luabin.h"

using namespace ast;

class Compiler
{
private:
    const char *chunckname = nullptr;
    IGenerator *gen;
    vector<Opcode> ops;
    vector<int> lines;
    vector<lbyte> vstack;
    vector<size_t> breaks;
    size_t stack_offset = 0;

    size_t hooksize = 0;
    size_t hookmax = 0;

    void hookpush();
    void hookpop();
    size_t len();
    void emit(Opcode op);
    void ops_flush();
    void edit_jmp(size_t opidx, size_t jmp_idx);
    void seti(size_t idx, lbyte b);
    size_t upval(fidx_t fidx, size_t offset, size_t hidx);
    void ops_push(Opcode op);
    void ops_push(Opcode op, int line);
    size_t const_number(lnumber n);
    size_t const_string(const char *s);
    size_t vstack_nearest_nil();
    MetaMemory *varmem(Noderef lvalue);
    void compile_node(Noderef node);
    void compile_decl(Noderef node);
    void compile_decl_var(Noderef node);
    void compile_decl_func(Noderef node);
    void compile_ret(Noderef node);
    void compile_block(Noderef node);
    void compile_primary(Noderef node, size_t expect);
    void compile_table(Noderef node);
    void compile_name(Noderef node);
    void compile_function(Noderef node);
    void compile_identifier(Noderef node);
    void compile_call(Noderef node, size_t expect);
    void compile_methcall(Noderef node, size_t expect);
    void compile_assignment(Noderef node);
    bool compile_lvalue(Noderef node);
    size_t compile_varlist(Noderef node);
    void compile_explist(Noderef node, size_t vcount);
    void compile_lvalue_primary(Noderef node);
    void compile_if(Noderef node);
    void compile_while(Noderef node);
    void compile_repeat(Noderef node);
    void compile_exp(Noderef node);
    void compile_logic(Noderef node);
    void compile_numeric_for(Noderef node);
    void compile_generic_for(Noderef node);
    void compile_generic_for_swap(size_t varcount);
    void compile_generic_for_swap_pair(size_t back_offset1, size_t back_offset2);
    void compile_break();
    void compile_goto(Noderef node);
    void compile_label(Noderef node);
    void compile_exp_e(Noderef node, size_t expect);
    void loop_start();
    void loop_end();
    size_t arglist_count(Noderef arglist);
    lbyte translate_token(TokenKind kind, bool bin);
    fidx_t compile(Noderef root, const char *chunckname = nullptr);
    void debug_info(size_t line);

public:
    Compiler(IGenerator *gen);
    fidx_t compile(Ast ast, const char *chunckname = nullptr);
};

#endif