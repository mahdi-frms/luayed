#include <iostream>
#include <cstring>
#include "lua.h"

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

bool runfile(const char *path)
{
    Lua lua;
    const char *text = readfile(path);
    lua.compile(text);
    lua.call(0, 1);
    lnumber num = lua.pop_number();
    std::cout << "result: " << num << "\n";
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
    std::cout << "PARSING DONE!\n";
    return 0;
}