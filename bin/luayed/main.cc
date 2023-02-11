#include <iostream>
#include <cstring>
#include <luadef.h>
#include <lua.h>

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

bool execute(Lua *lua, size_t argc = 0)
{
    lua->call(argc, 1);
    if (lua->has_error())
    {
        lua->push_error();
        std::cerr << "lua: " << lua->peek_string();
        lua->pop();
        return false;
    }
    else
    {
        return true;
    }
}

bool runfile(int argc, char **argv)
{
    const char *script = argv[1];
    Lua lua;
    string text = readfile(script);
    string errors;
    bool rsl;
    if (lua.compile(text.c_str(), errors, script) == LUA_COMPILE_RESULT_OK)
    {
        for (int i = 2; i < argc; i++)
            lua.push_string(argv[i]);
        rsl = execute(&lua, argc - 2);
    }
    else
    {
        std::cerr << errors;
        rsl = false;
    }
    return rsl;
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
        runfile(argc, argv);
    }
    return 0;
}