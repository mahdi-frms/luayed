#include <iostream>
#include "parser.hpp"

string readfile(const char *path)
{
    string text = "";
    int bsize = 1024;
    FILE *file = fopen(path, "r");
    char buffer[bsize + 1];
    while (true)
    {
        int rsl = fread(buffer, 1, bsize, file);
        buffer[rsl] = '\0';
        text += string(buffer);
        if (rsl < bsize)
            break;
    }
    fclose(file);
    return text;
}

bool parse(const char *path)
{
    bool silence = true;
    bool parse = true;
    string text = readfile(path);
    printf("===== %s =====\n", path);
    Lexer lxr = Lexer(text);
    if (parse)
    {
        Parser parser = Parser(lxr);
        ast::Noderef tree = parser.parse().get_root();
        if (tree != nullptr && !silence)
        {
            printf("%s", tree->to_string().c_str());
        }
        return tree != nullptr;
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
            printf("--> %s (%s) [%lu,%lu]\n", tkn.text.c_str(), token_kind_stringify(tkn.kind).c_str(), tkn.line + 1, tkn.offset + 1);
        }
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
            return 1;
        }
    }
    printf("PARSING DONE!\n");
    return 0;
}