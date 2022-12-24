#include <iostream>
#include <cstring>
#include "lua.hpp"

char *readfile(const char *path)
{
    string str = "";
    int bsize = 1024;
    FILE *file = fopen(path, "r");
    char buffer[bsize + 1];
    while (true)
    {
        int rsl = fread(buffer, 1, bsize, file);
        buffer[rsl] = '\0';
        str += string(buffer);
        if (rsl < bsize)
            break;
    }
    fclose(file);
    char *text = new char[str.size() + 1];
    text[str.size()] = '\0';
    strcpy(text, str.c_str());
    return text;
}

void print_fns(LuaRuntime *rt)
{
    for (size_t i = 1; i < rt->functable.size(); i++)
    {
        std::cout << "--------------- " << rt->functable[i]->fidx << " ---------------\n";
        std::cout << rt->functable[i]->stringify();
    }
}

// void previous_main()
// {
//     Lexer lxr = Lexer(text);
//     if (parse)
//     {
//         Parser parser = Parser((ILexer *)&lxr);
//         ast::Ast tree = parser.parse();
//         ast::Noderef root = tree.root();
//         if (root != nullptr)
//         {
//             SemanticAnalyzer sem = SemanticAnalyzer(tree);
//             sem.analyze();
//             if (compile)
//             {
//                 LuaRuntime lua;
//                 LuaGenerator gen(&lua);
//                 Compiler compiler((IGenerator *)&gen);
//                 compiler.compile(tree);
//                 if (!silence)
//                 {
//                     print_fns(&lua);
//                 }
//             }
//             if (!silence && !compile)
//             {
//                 std::cout << root->to_string();
//                 std::cout.flush();
//             }
//         }
//         else
//         {
//             std::cerr << parser.get_error();
//         }
//         tree.destroy();
//         delete[] text;
//         return root != nullptr;
//     }
//     else
//     {
//         for (;;)
//         {
//             Token tkn = lxr.next();
//             if (tkn.kind == TokenKind::Eof)
//             {
//                 break;
//             }
//             if (!silence)
//             {
//                 if (tkn.kind == TokenKind::Error)
//                 {
//                     std::cout << lxr.get_error();
//                 }
//                 else
//                 {
//                     printf("--> %s (%s) [%lu,%lu]\n", tkn.text().c_str(), token_kind_stringify(tkn.kind).c_str(), tkn.line + 1, tkn.offset + 1);
//                 }
//             }
//         }
//         delete[] text;
//         return true;
//     }
// }

bool runfile(const char *path)
{
    Lua lua;
    const char *text = readfile(path);
    lua.compile(text);
    lua.call(0, 0);
    return true; // todo: must check for errors
}

int main(int argc, char **argv)
{
    if (argc == 1)
        return 1;
    for (int i = 1; i < argc; i++)
    {
        if (!runfile(argv[i]))
        {
            // return 1;
        }
    }
    printf("PARSING DONE!\n");
    return 0;
}