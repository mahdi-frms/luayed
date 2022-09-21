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

// int main(int argc, char **argv)
// {
//     string text = readfile(argv[1]);
//     printf("%s\n", text.c_str());
//     Lexer lxr = Lexer(text);
//     for (;;)
//     {
//         Token tkn = lxr.next();
//         if (tkn.kind == TokenKind::Eof)
//         {
//             break;
//         }
//         printf("--> %s (%s) [%lu,%lu]\n", tkn.text.c_str(), token_kind_stringify(tkn.kind).c_str(), tkn.line, tkn.offset);
//     }
//     return 0;
// }

int main(int argc, char **argv)
{
    string text = readfile(argv[1]);
    text.erase(text.begin());
    text.erase(text.begin());
    text.erase(text.begin());
    text.erase(text.begin());
    printf("%s\n", text.c_str());
    Lexer lxr = Lexer(text);
    Parser parser = Parser(lxr);
    ast::Noderef tree = parser.parse().get_root();
    if (tree != nullptr)
    {
        printf("%s", tree->to_string().c_str());
    }
    return 0;
}