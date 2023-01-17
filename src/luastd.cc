#include "luastd.h"
#include "lstrep.h"

string luastd::luavalue_to_string(Lua *lua)
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

size_t luastd::print(Lua *lua)
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

size_t luastd::unpack(Lua *lua)
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

size_t luastd::tostring(Lua *lua)
{
    if (lua->top())
    {
        string str = luastd::luavalue_to_string(lua);
        lua->push_string(str.c_str());
    }
    else
    {
        lua->push_nil();
    }
    return 1;
}

void luastd::libinit(Lua *lua)
{
    libcpp_init(lua);
    liblua_init(lua);
}
void luastd::libcpp_init(Lua *lua)
{
    lua->push_string("print");
    lua->push_cppfn(luastd::print);
    lua->set_global();

    lua->push_string("tostring");
    lua->push_cppfn(luastd::tostring);
    lua->set_global();

    lua->push_string("unpack");
    lua->push_cppfn(luastd::unpack);
    lua->set_global();
}
void luastd::liblua_init(Lua *lua)
{
    string err;
    lua->compile(liblua_code, err);
    lua->call(0, 1);
}