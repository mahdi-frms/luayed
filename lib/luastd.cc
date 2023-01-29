#include "luastd.h"
#include <lstrep.h>

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
        str = lua->pop_boolean() ? "true" : "false";
    }
    else if (k == LUA_TYPE_NUMBER)
    {
        poped = true;
        str = to_string(lua->pop_number());
    }
    else if (k == LUA_TYPE_STRING)
    {
        str = lua->peek_string();
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

size_t luastd::type(Lua *lua)
{
    const char *arg_error = "bad argument to type function";
    if (lua->top() == 0)
    {
        lua->push_string(arg_error);
        lua->pop_error();
    }
    lua->fetch_local(0);
    LuaType t = (LuaType)lua->kind();
    lua->push_string(to_string(t).c_str());
    lua->store_local(0);
    while (lua->top() > 1)
        lua->pop();
    return 1;
}
size_t luastd::error(Lua *lua)
{
    if (lua->top() == 0)
        lua->push_nil();
    lua->fetch_local(0);
    lua->pop_error();
    while (lua->top() > 1)
        lua->pop();
    return 0;
}
size_t luastd::pcall(Lua *lua)
{
    const char *arg_error = "bad argument to pcall function";
    if (lua->top() == 0)
    {
        lua->push_string(arg_error);
        lua->pop_error();
        return 0;
    }
    lua->fetch_local(0);
    if (lua->kind() != LUA_TYPE_FUNCTION)
    {
        lua->push_string(arg_error);
        lua->pop_error();
        return 0;
    }
    lua->pop();
    lua->call(lua->top() - 1, 0);
    if (lua->has_error())
    {
        lua->push_boolean(false);
        lua->push_error();
        return 2;
    }
    else
    {
        lua->push_boolean(true);
        lua->insert(0);
        return lua->top();
    }
}
size_t luastd::load(Lua *lua)
{
    const char *arg_error = "bad argument to load function";
    if (lua->top())
    {
        lua->fetch_local(0);
        if (lua->kind() == LUA_TYPE_STRING)
        {
            const char *chunckname = nullptr;
            if (lua->top() >= 2)
            {
                lua->fetch_local(1);
                if (lua->kind() != LUA_TYPE_STRING)
                {
                    lua->push_string(arg_error);
                    lua->pop_error();
                    return 0;
                }
                chunckname = lua->peek_string();
                lua->pop();
            }
            const char *src = lua->peek_string();
            string compile_errors;
            int rsl = lua->compile(src, compile_errors, chunckname);
            if (rsl == 0)
            {
                lua->store_local(0);
                while (lua->top() > 1)
                    lua->pop();
                return 1;
            }
            else
            {
                while (lua->top())
                    lua->pop();
                lua->push_nil();
                lua->push_string(compile_errors.c_str());
                return 2;
            }
        }
        else
        {
            lua->push_string(arg_error);
            lua->pop_error();
        }
    }
    else
    {
        lua->push_string(arg_error);
        lua->pop_error();
    }
    return 0;
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

    lua->push_string("load");
    lua->push_cppfn(luastd::load);
    lua->set_global();

    lua->push_string("type");
    lua->push_cppfn(luastd::type);
    lua->set_global();

    lua->push_string("error");
    lua->push_cppfn(luastd::error);
    lua->set_global();

    lua->push_string("pcall");
    lua->push_cppfn(luastd::pcall);
    lua->set_global();
}
void luastd::liblua_init(Lua *lua)
{
    string err;
    lua->compile(liblua_code, err);
    lua->call(0, 1);
}