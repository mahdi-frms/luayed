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

struct Scope
{
    Noderef node;
    Varmap map;
    size_t stack_size;
    bool variadic;
};

class SemanticAnalyzer
{
private:
    vector<Scope> scopes;
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
    Scope &curscope();
    void finalize();

public:
    SemanticAnalyzer(Ast ast);
    vector<SemanticError> analyze();
};

#endif