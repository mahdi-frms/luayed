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

bool execute(Lua *lua)
{
    lua->call(0, 1);
    if (lua->has_error())
    {
        lua->push_error();
        std::cerr << "lua: " << luastd::luavalue_to_string(lua) << "\n";
        return false;
    }
    else
    {
        return true;
    }
}

bool runfile(const char *path)
{
    Lua lua;
    string text = readfile(path);
    string errors;
    if (lua.compile(text.c_str(), errors, path) == LUA_COMPILE_RESULT_OK)
    {
        return execute(&lua);
    }
    else
    {
        std::cerr << errors;
        return false;
    }
}

void repl()
{
    const char *chunkname = "[line]";
    Lua lua;
    while (1)
    {
        string input;
        std::cout << "> ";
        std::flush(std::cout);
        if (!std::getline(std::cin, input))
            break;
        if (input.length() == 0)
            continue;
        string exp = string("print(") + input + string(")");
        string errors;
        int rsl = lua.compile(exp.c_str(), errors, chunkname);
        if (rsl == LUA_COMPILE_RESULT_FAILED)
        {
            rsl = lua.compile(input.c_str(), errors, chunkname);
        }
        if (rsl == LUA_COMPILE_RESULT_FAILED)
        {
            std::cerr << errors << "\n";
        }
        else
        {
            execute(&lua);
        }
    }
    std::cout << "\n";
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        repl();
    }
    else
    {
        for (int i = 1; i < argc; i++)
            runfile(argv[i]);
    }
    return 0;
}