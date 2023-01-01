#ifndef RESOLVE_h
#define RESOLVE_h

#include "ast.h"
#include <map>

using namespace ast;

typedef std::map<std::string, Noderef> Varmap;

class SemanticAnalyzer
{
private:
    Varmap labels;
    vector<Noderef> gotolist;
    vector<LError> errors;
    Ast ast;
    Noderef current;

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
    MetaScope *curscope();
    Varmap &curmap();

public:
    SemanticAnalyzer(Ast ast);
    vector<LError> analyze();
};

#endif