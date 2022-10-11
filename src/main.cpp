#include <iostream>
#include <cstdlib>
#include <cstring>
#include "parser.hpp"
#include "resolve.hpp"

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
    char *text = (char *)malloc(str.size() + 1);
    text[str.size()] = '\0';
    strcpy(text, str.c_str());
    return text;
}

bool parse(const char *path)
{
    bool silence = false;
    bool parse = false;
    char *text = readfile(path);
    printf("===== %s =====\n", path);
    Lexer lxr = Lexer(text);
    if (parse)
    {
        Parser parser = Parser(lxr);
        ast::Ast tree = parser.parse();
        ast::Noderef root = tree.root();
        if (root != nullptr)
        {
            SemanticAnalyzer sem = SemanticAnalyzer(tree);
            sem.analyze();
            if (!silence)
            {
                std::cout << root->to_string();
                std::cout.flush();
            }
        }
        tree.destroy();
        free(text);
        return root != nullptr;
    }
    else
    {
        for (;;)
        {
            Token tkn = lxr.next();
            if (tkn.kind == TokenKind::Eof)
            {
                break;
            }
            if (tkn.kind == TokenKind::Eof)
            {
                break;
            }
            if (!silence)
                printf("--> %s (%s) [%lu,%lu]\n", tkn.text().c_str(), token_kind_stringify(tkn.kind).c_str(), tkn.line + 1, tkn.offset + 1);
        }
        free(text);
        return true;
    }
}

int main(int argc, char **argv)
{
    if (argc == 1)
        return 1;
    for (int i = 1; i < argc; i++)
    {
        if (!parse(argv[i]))
        {
            // return 1;
        }
    }
    printf("PARSING DONE!\n");
    return 0;
}