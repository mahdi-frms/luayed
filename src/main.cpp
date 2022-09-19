#include <iostream>
#include "lexer.hpp"

int main()
{
    string text = "local a = 3";
    Lexer lxr = Lexer(text);
    vector<Token> tkns = lxr.drain();
    for (size_t i = 0; i < tkns.size(); i++)
    {
        printf("%s\n", tkns[i].text.c_str());
    }
    return 0;
}