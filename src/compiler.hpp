#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "ast.hpp"
#include "runtime.hpp"

using namespace ast;

struct Opcode
{
    lbyte count;
    lbyte bytes[5];

    Opcode(lbyte op, size_t idx);
    Opcode(lbyte op, size_t idx1, size_t idx2);
    Opcode(lbyte op);
};

class Compiler
{
private:
    Lua *rt;
    vector<Opcode> ops;
    vector<lbyte> vstack;
    vector<size_t> breaks;
    vector<Upvalue> upvalues;
    vector<lbyte> text;
    vector<LuaValue> rodata;

    bool method = false;

    size_t len();
    void emit(Opcode op);
    void ops_flush();
    void ops_push(Opcode op);
    size_t constant(LuaValue val);
    size_t const_number(lnumber n);
    size_t const_string(const char *s);
    size_t vstack_nearest_nil();
    MetaMemory *varmem(Noderef lvalue);
    void compile_node(Noderef node);
    void compile_decl(Noderef node);
    void compile_ret(Noderef node);
    void compile_block(Noderef node);
    void compile_primary(Noderef node, size_t expect);
    void compile_table(Noderef node);
    void compile_name(Noderef node);
    void compile_function(Noderef node);
    void compile_identifier(Noderef node);
    void compile_call(Noderef node, size_t expect);
    void compile_methcall(Noderef node, size_t expect);
    void compile_assignment(Noderef node, bool attrib);
    void compile_lvalue(Noderef node);
    void compile_varlist(Noderef node, bool attrib);
    void compile_explist(Noderef node, size_t vcount);
    void compile_lvalue_primary(Noderef node);
    void compile_if(Noderef node);
    void compile_while(Noderef node);
    void compile_repeat(Noderef node);
    void compile_exp(Noderef node);
    void compile_logic(Noderef node);
    void compile_numeric_for(Noderef node);
    void compile_generic_for(Noderef node);
    void compile_break();
    void compile_exp_e(Noderef node, size_t expect);
    void loop_start();
    void loop_end();
    size_t arglist_count(Noderef arglist);
    lbyte translate_token(TokenKind kind, bool bin);
    Lfunction *compile(Noderef root);

public:
    Compiler(Lua *rt);
    void compile(Ast ast);
};

#endif