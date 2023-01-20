#include <iostream>
#include <cstring>
#include "lua.h"
#include "lstrep.h"
#include "luastd.h"

string readfile(const char *path)
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
    return str;
}

bool runfile(const char *path)
{
    Lua lua;
    string text = readfile(path);
    string errors;
    if (lua.compile(text.c_str(), errors) == LUA_COMPILE_RESULT_OK)
    {
        lua.call(0, 1);
        if (lua.has_error())
        {
            lua.push_error();
            std::cerr << "lua: " << luastd::luavalue_to_string(&lua) << "\n";
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        std::cerr << errors;
        return false;
    }
}

int main(int argc, char **argv)
{
    if (argc == 1)
        return 1;
    runfile(argv[1]);
    return 0;
}