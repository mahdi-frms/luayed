#include <iostream>
#include "lexer.hpp"

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

int main(int argc, char **argv)
{
    string text = readfile(argv[1]);
    printf("%s\n", text.c_str());
    Lexer lxr = Lexer(text);
    for (;;)
    {
        Token tkn = lxr.next();
        if (tkn.kind == TokenKind::Eof)
        {
            break;
        }
        printf("--> %s [%lu,%lu]\n", tkn.text.c_str(), tkn.line, tkn.offset);
    }
    return 0;
}