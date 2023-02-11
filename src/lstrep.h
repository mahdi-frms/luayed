#ifndef LSTREP_H
#define LSTREP_H

#include "luadef.h"
#include "token.h"
#include "ast.h"

namespace luayed
{
    string to_string(const TokenKind &tk);
    string to_string(const Token &t, const char *source);
    string to_string(const ast::NodeKind &nk);
    string to_string(const ast::Noderef &n, const char *source);
    string to_string(const vector<lbyte> &bin);
    string to_string(const LuaType &lt);
    string to_string(const LuaValue &lv);
    string to_string(const Lerror &err);
    string to_string(const Lerror &err, bool pure);
    string to_string(const vector<LuaValue> &vv);
    string to_string(lnumber n);
    string to_string(const Opcode &opcode);

    string to_string(const lbyte *text, size_t codelen);
};

std::ostream &operator<<(std::ostream &os, const luayed::TokenKind &tk);
std::ostream &operator<<(std::ostream &os, const luayed::Token &t);
std::ostream &operator<<(std::ostream &os, const luayed::ast::NodeKind &nk);
std::ostream &operator<<(std::ostream &os, const luayed::ast::Noderef &n);
std::ostream &operator<<(std::ostream &os, const vector<luayed::lbyte> &tk);
std::ostream &operator<<(std::ostream &os, const luayed::LuaType &lt);
std::ostream &operator<<(std::ostream &os, const luayed::LuaValue &lv);
std::ostream &operator<<(std::ostream &os, const luayed::Lerror &err);
std::ostream &operator<<(std::ostream &os, const vector<luayed::LuaValue> &vv);
std::ostream &operator<<(std::ostream &os, const luayed::Opcode &opcode);

#endif