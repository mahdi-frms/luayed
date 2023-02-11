#ifndef RESOLVE_h
#define RESOLVE_h

#include "ast.h"

#define map(N) (*((Varmap *)(N->metadata_scope()->map)))

namespace luayed
{
    using namespace ast;

    class Resolver
    {
    private:
        vector<Lerror> errors;
        Ast ast;
        Noderef current = nullptr;
        size_t stack_ptr = 0;
        size_t hook_ptr = 0;
        const char *source;

        MetaGoto *metadata_goto(Noderef node);
        MetaLabel *metadata_label(Noderef node);
        MetaDeclaration *metadata_decl(Noderef node);
        MetaMemory *metadata_memory(Noderef node);
        MetaScope *metadata_scope(Noderef node);
        MetaSelf *metadata_self(Noderef node);

        void analyze_node(Noderef node);
        void analyze_var_decl(Noderef node);
        void analyze_identifier(Noderef node);
        void analyze_etc(Noderef node);
        void analyze_break(Noderef node);
        void analyze_label(Noderef node);
        void analyze_goto(Noderef node);
        void analyze_declaration(Noderef node);
        void reference(Noderef node, Noderef dec, bool func_past);
        void self_ref(Noderef node);
        void link(Noderef go_to, Noderef label);
        void link_labels();
        MetaScope *curscope();
        Varmap &curmap();

    public:
        Resolver(Ast ast, const char *source);
        vector<Lerror> analyze();
    };
};

#endif