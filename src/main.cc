#include <iostream>
#include <cstring>
#include "lua.h"
#include "lstrep.h"

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

string luavalue_to_string(Lua *lua)
{
    int k = lua->kind();
    bool poped = false;
    string str;
    if (k == LUA_TYPE_NIL)
        str = "nil";
    else if (k == LUA_TYPE_BOOLEAN)
    {
        poped = true;
        str = std::to_string(lua->pop_boolean());
    }
    else if (k == LUA_TYPE_NUMBER)
    {
        poped = true;
        str = to_string(lua->pop_number());
    }
    else if (k == LUA_TYPE_STRING)
    {
        poped = true;
        str = lua->pop_string();
    }
    else if (k == LUA_TYPE_TABLE)
        str = "[table]";
    else if (k == LUA_TYPE_FUNCTION)
        str = "[function]";
    if (!poped)
        lua->pop();
    return str;
}

size_t lua_std_print(Lua *lua)
{
    for (size_t i = 0; i < lua->top(); i++)
    {
        lua->fetch_local(i);
        std::cout << luavalue_to_string(lua);
        std::cout << "\t";
    }
    std::cout << "\n";
    return 0;
}

size_t lua_std_unpack(Lua *lua)
{
    size_t count = 1;
    while (true)
    {
        lua->fetch_local(-1); // table
        lua->push_number(count);
        lua->get_table();
        if (lua->kind() == LUA_TYPE_NIL)
        {
            lua->pop();
            break;
        }
        count++;
        lua->fetch_local(-2); // table
        lua->fetch_local(-2); // value
        lua->store_local(-3);
        lua->store_local(-1);
    }
    lua->pop();
    return count - 1;
}

size_t lua_std_to_string(Lua *lua)
{
    if (lua->top())
    {
        string str = luavalue_to_string(lua);
        lua->push_string(str.c_str());
    }
    else
    {
        lua->push_nil();
    }
    return 1;
}

void init_luastd(Lua *lua)
{
    lua->push_string("print");
    lua->push_cppfn(lua_std_print);
    lua->set_global();

    lua->push_string("tostring");
    lua->push_cppfn(lua_std_to_string);
    lua->set_global();

    lua->push_string("unpack");
    lua->push_cppfn(lua_std_unpack);
    lua->set_global();
}

bool runfile(const char *path)
{
    Lua lua;
    init_luastd(&lua);
    const char *text = readfile(path);
    string errors;
    if (lua.compile(text, errors) == LUA_COMPILE_RESULT_OK)
    {
        lua.call(0, 1);
        std::cout << "-------------------\n";
        if (lua.has_error())
        {
            std::cout << "error: ";
            lua.push_error();
        }
        else
        {
            std::cout << "result: ";
        }
        std::cout << luavalue_to_string(&lua);
        std::cout << "\n";
        return true; // todo: must check for runtime errors
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
    for (int i = 1; i < argc; i++)
    {
        if (!runfile(argv[i]))
        {
            // return 1;
        }
    }
    return 0;
}