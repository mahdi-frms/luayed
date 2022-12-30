#ifndef LSTREP_H
#define LSTREP_H

#include "luadef.h"
#include "token.h"
#include "ast.h"

string to_string(const TokenKind &tk);
string to_string(const Token &t);
string to_string(const ast::NodeKind &nk);
string to_string(const ast::Noderef &n);
string to_string(const vector<lbyte> &bin);
string to_string(const LuaType &lt);
string to_string(const LuaValue &lv);
string to_string(const LError &err);

string to_string(const lbyte *text, size_t codelen);

std::ostream &operator<<(std::ostream &os, const TokenKind &tk);
std::ostream &operator<<(std::ostream &os, const Token &t);
std::ostream &operator<<(std::ostream &os, const ast::NodeKind &nk);
std::ostream &operator<<(std::ostream &os, const ast::Noderef &n);
std::ostream &operator<<(std::ostream &os, const vector<lbyte> &tk);
std::ostream &operator<<(std::ostream &os, const LuaType &lt);
std::ostream &operator<<(std::ostream &os, const LuaValue &lv);
std::ostream &operator<<(std::ostream &os, const LError &err);

#endif