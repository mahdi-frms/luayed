#ifndef RESOLVE_HPP
#define RESOLVE_HPP

#include "ast.hpp"
#include <map>

using namespace ast;

typedef std::map<std::string, Noderef> Varmap;

struct SemanticError
{
    Noderef node;
    string text;
};

class SemanticAnalyzer
{
private:
    Varmap labels;
    vector<Noderef> decls;
    vector<Noderef> gotolist;
    vector<SemanticError> errors;
    Ast ast;
    Noderef current;
    size_t fn_idx;

    void analyze_node(Noderef node);
    void analyze_var_decl(Noderef node);
    void analyze_identifier(Noderef node);
    void analyze_etc(Noderef node);
    void analyze_break(Noderef node);
    void analyze_label(Noderef node);
    void analyze_declaration(Noderef node);
    void reference(Noderef node, Noderef dec, bool func_past);
    void self_ref(Noderef node);
    void finalize();
    void fix_offsets();
    MetaScope *curscope();
    Varmap &curmap();

public:
    SemanticAnalyzer(Ast ast);
    vector<SemanticError> analyze();
};

#endif