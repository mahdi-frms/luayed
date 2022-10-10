#ifndef RESOLVE_HPP
#define RESOLVE_HPP

#include "ast.hpp"
#include <unordered_map>

using namespace ast;

typedef std::unordered_map<std::string, Noderef> Varmap;

struct SemanticError
{
    Noderef node;
    string text;
};

class SemanticAnalyzer
{
private:
    vector<Varmap> maps;
    vector<Noderef> nodes;
    Varmap labels;
    vector<Noderef> gotolist;
    vector<SemanticError> errors;
    Ast ast;

    void analyze_node(Noderef node);
    void analyze_var_decl(Noderef node);
    void analyze_identifier(Noderef node);
    void analyze_etc(Noderef node);
    void analyze_break(Noderef node);
    void analyze_label(Noderef node);
    void analyze_declaration(Noderef node);
    void finalize();

public:
    SemanticAnalyzer(Ast ast);
    vector<SemanticError> analyze();
};

#endif