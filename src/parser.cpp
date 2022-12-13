#include "parser.hpp"
#include <utility>
using namespace ast;

#define BINPWR(L, R) (L * 256 + R)

constexpr uint16_t binpwr[21] = {
    // Plus = 0x0400,
    BINPWR(27, 28),
    // Multiply = 0x0401,
    BINPWR(29, 30),
    // FloatDivision = 0x0402,
    BINPWR(29, 30),
    // FloorDivision = 0x0403,
    BINPWR(29, 30),
    // Modulo = 0x0404,
    BINPWR(29, 30),
    // Power = 0x0405,
    BINPWR(34, 33),
    // BinAnd = 0x0406,
    BINPWR(21, 22),
    // BinOr = 0x0407,
    BINPWR(17, 18),
    // RightShift = 0x0408,
    BINPWR(23, 24),
    // LeftShift = 0x0409,
    BINPWR(23, 24),
    // DotDot = 0x040a,
    BINPWR(26, 25),
    // Less = 0x040b,
    BINPWR(15, 16),
    // LessEqual = 0x040c,
    BINPWR(15, 16),
    // Greater = 0x040d,
    BINPWR(15, 16),
    // GreaterEqual = 0x040e,
    BINPWR(15, 16),
    // EqualEqual = 0x040f,
    BINPWR(15, 16),
    // NotEqual = 0x0410,
    BINPWR(15, 16),
    // And = 0x0411,
    BINPWR(13, 14),
    // Or = 0x0412,
    BINPWR(11, 12),
    // Minus = 0x0c13,
    BINPWR(27, 28),
    // Negate = 0x0c14,
    BINPWR(19, 20),
};

uint8_t check_prefix(TokenKind kind)
{
    if (TOKEN_IS_PREFIX(kind))
        return 31;
    return 255;
}

bool is_var(Noderef node)
{
    if (node->get_kind() == NodeKind::Index || node->get_kind() == NodeKind::Property)
        return true;
    if (node->get_kind() != NodeKind::Primary)
        return false;
    return node->get_token().kind == TokenKind::Identifier;
}

uint8_t check_postfix(TokenKind kind)
{
    if (TOKEN_IS_POSTFIX(kind))
        return 35;
    return 255;
}

Parser::Parser(ILexer *lexer) : lexer(lexer), current(token_none()), ahead(token_none())
{
    this->current = this->lexer->next();
    if (this->current.kind != TokenKind::Eof)
    {
        this->ahead = this->lexer->next();
    }
}

/*
    11 12       or
    13 14       and
    15 16       < > <= >= ~= ==
    17 18       |
    19 20       ~
    21 22       &
    23 24       >> <<
    26 25       ..
    27 28       + -
    29 30       * /
    31          not -(unary) ~(unary)
    34 33       ^
*/
Noderef Parser::expr()
{
    return this->expr_p(0);
}

Token Parser::consume(TokenKind kind)
{
    Token t = this->pop();
    if (t.kind != kind)
    {
        this->error(string("expected '") +
                        token_kind_stringify(kind) +
                        "'",
                    t);
    }
    return t;
}

Noderef Parser::make(NodeKind kind)
{
    return new Node(token_none(), kind);
}
Noderef Parser::make(vector<Noderef> &nodes, NodeKind kind)
{
    Noderef *children = new Noderef[nodes.size()];
    for (size_t i = 0; i < nodes.size(); i++)
        children[i] = nodes[i];
    return new Node(children, nodes.size(), kind);
}
Noderef Parser::make(Token token, NodeKind kind)
{
    return new Node(token, kind);
}
Noderef Parser::make(Noderef c1, NodeKind kind)
{
    Noderef *nodes = new Noderef[1];
    nodes[0] = c1;
    return new Node(nodes, 1, kind);
}
Noderef Parser::make(Noderef c1, Noderef c2, NodeKind kind)
{
    Noderef *nodes = new Noderef[2];
    nodes[0] = c1;
    nodes[1] = c2;
    return new Node(nodes, 2, kind);
}
Noderef Parser::make(Noderef c1, Noderef c2, Noderef c3, NodeKind kind)
{
    Noderef *nodes = new Noderef[3];
    nodes[0] = c1;
    nodes[1] = c2;
    nodes[2] = c3;
    return new Node(nodes, 3, kind);
}
Noderef Parser::make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, NodeKind kind)
{
    Noderef *nodes = new Noderef[4];
    nodes[0] = c1;
    nodes[1] = c2;
    nodes[2] = c3;
    nodes[3] = c4;
    return new Node(nodes, 4, kind);
}
Noderef Parser::make(Noderef c1, Noderef c2, Noderef c3, Noderef c4, Noderef c5, NodeKind kind)
{
    Noderef *nodes = new Noderef[5];
    nodes[0] = c1;
    nodes[1] = c2;
    nodes[2] = c3;
    nodes[3] = c4;
    nodes[4] = c5;
    return new Node(nodes, 5, kind);
}

Noderef Parser::id_field()
{
    Noderef id = this->make(this->pop(), NodeKind::Name);
    this->consume(TokenKind::Equal);
    return this->make(id, this->expr(), NodeKind::IdField);
}

Noderef Parser::expr_field()
{
    this->pop(); // [
    Noderef field = this->expr();
    this->consume(TokenKind::RightBracket);
    this->consume(TokenKind::Equal);
    return this->make(field, this->expr(), NodeKind::ExprField);
}

Noderef Parser::table()
{
    vector<Noderef> items;
    while (true)
    {
        Token t = this->peek();
        if (t.kind == TokenKind::RightBrace)
        {
            this->pop();
            break;
        }
        else if (t.kind == TokenKind::Identifier)
        {
            if (this->look_ahead().kind == TokenKind::Equal)
            {
                items.push_back(this->id_field());
            }
            else
            {
                items.push_back(this->expr());
            }
        }
        else if (t.kind == TokenKind::LeftBracket)
        {
            items.push_back(this->expr_field());
        }
        else
        {
            items.push_back(this->expr());
        }

        t = this->peek();
        if (t.kind != TokenKind::Comma &&
            t.kind != TokenKind::Semicolon &&
            t.kind != TokenKind::RightBrace)
        {
            this->error(string("expected ',' or ';'"), t);
        }
        else if (t.kind != TokenKind::RightBrace)
        {
            this->pop();
        }
    }
    return this->make(items, NodeKind::Table);
}

Noderef Parser::arglist()
{
    vector<Noderef> args;
    while (this->peek().kind != TokenKind::RightParen)
    {
        args.push_back(this->expr());
        if (this->peek().kind == TokenKind::RightParen)
        {
            break;
        }
        this->consume(TokenKind::Comma);
    }
    this->pop();
    return this->make(args, NodeKind::Explist);
}

Noderef Parser::varlist(Noderef var)
{
    vector<Noderef> items;
    items.push_back(var);
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        Noderef var = this->expr();
        if (!is_var(var))
            this->error("expected variable", this->peek());
        items.push_back(var);
    }
    return this->make(items, NodeKind::Explist);
}

Noderef Parser::explist()
{
    vector<Noderef> items;
    items.push_back(this->expr());
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        items.push_back(this->expr());
    }
    return this->make(items, NodeKind::Explist);
}

Noderef Parser::generic_for_stmt(Token identifier)
{
    vector<Noderef> namelist;
    namelist.push_back(this->make(this->make(identifier, NodeKind::Name), NodeKind::VarDecl));
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        namelist.push_back(this->make(this->make(this->consume(TokenKind::Identifier), NodeKind::Name), NodeKind::VarDecl));
    }
    this->consume(TokenKind::In);
    Noderef explist = this->explist();
    this->consume(TokenKind::Do);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return this->make(this->make(namelist, NodeKind::NameList), explist, block, NodeKind::GenericFor);
}

Noderef Parser::numeric_for_stmt(Token identifier)
{
    this->pop(); // =
    Noderef from = this->expr();
    this->consume(TokenKind::Comma);
    Noderef to = this->expr();
    Noderef step = nullptr;
    if (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        step = this->expr();
    }
    this->consume(TokenKind::Do);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    if (step)
        return this->make(this->make(this->make(identifier, NodeKind::Name), NodeKind::VarDecl), from, to, step, block, NodeKind::NumericFor);
    else
        return this->make(this->make(this->make(identifier, NodeKind::Name), NodeKind::VarDecl), from, to, block, NodeKind::NumericFor);
}

Noderef Parser::while_stmt()
{
    this->pop();
    Noderef expr = this->expr();
    this->consume(TokenKind::Do);
    Noderef blk = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return this->make(expr, blk, NodeKind::WhileStmt);
}
Noderef Parser::repeat_stmt()
{
    this->pop();
    Noderef blk = this->block(BlockEnd::Until);
    this->consume(TokenKind::Until);
    Noderef expr = this->expr();
    return this->make(blk, expr, NodeKind::RepeatStmt);
}
Noderef Parser::if_stmt()
{
    vector<Noderef> clauses;
    this->pop();
    Noderef expr = this->expr();
    this->consume(TokenKind::Then);
    Noderef block = this->block(BlockEnd::Else);
    clauses.push_back(this->make(expr, block, NodeKind::IfClause));
    while (this->peek().kind == TokenKind::ElseIf)
    {
        this->pop();
        expr = this->expr();
        this->consume(TokenKind::Then);
        block = this->block(BlockEnd::Else);
        clauses.push_back(this->make(expr, block, NodeKind::ElseIfClause));
    }
    if (this->peek().kind == TokenKind::Else)
    {
        this->pop();
        clauses.push_back(this->make(this->block(BlockEnd::End), NodeKind::ElseClause));
    }
    this->consume(TokenKind::End);
    return this->make(clauses, NodeKind::IfStmt);
}

Noderef Parser::statement()
{
    if (!TOKEN_IS_STATEMENT(this->peek().kind))
    {
        Noderef s = this->expr();
        if (s->get_kind() == NodeKind::Call || s->get_kind() == NodeKind::MethodCall)
            return this->make(s, NodeKind::CallStmt);

        else if (is_var(s))
        {
            Noderef vars = this->varlist(s);
            this->consume(TokenKind::Equal);
            Noderef vals = this->explist();
            return this->make(vars, vals, NodeKind::AssignStmt);
        }
        else
        {
            this->error(string("unexpected expression before"), this->peek());
        }
    }
    if (this->peek().kind == TokenKind::Semicolon)
    {
        this->pop();
        return nullptr;
    }
    if (this->peek().kind == TokenKind::ColonColon)
    {
        this->pop();
        Token id = this->consume(TokenKind::Identifier);
        this->consume(TokenKind::ColonColon);
        return this->make(id, NodeKind::LabelStmt);
    }
    if (this->peek().kind == TokenKind::Break)
    {
        this->pop();
        return this->make(NodeKind::BreakStmt);
    }
    if (this->peek().kind == TokenKind::Goto)
    {
        this->pop();
        return this->make(this->consume(TokenKind::Identifier), NodeKind::GotoStmt);
    }
    if (this->peek().kind == TokenKind::Do)
    {
        this->pop();
        Noderef blk = this->block(BlockEnd::End);
        this->consume(TokenKind::End);
        return blk;
    }
    if (this->peek().kind == TokenKind::While)
        return this->while_stmt();
    if (this->peek().kind == TokenKind::Repeat)
        return this->repeat_stmt();
    if (this->peek().kind == TokenKind::If)
        return this->if_stmt();
    if (this->peek().kind == TokenKind::Local)
    {
        this->pop();
        if (this->peek().kind == TokenKind::Function)
        {
            this->pop();
            Token t = this->consume(TokenKind::Identifier);

            return this->make(
                this->make(
                    this->make(
                        this->make(t, NodeKind::Name), NodeKind::VarDecl),
                    NodeKind::VarList),
                this->make(this->function_body(false), NodeKind::Explist), NodeKind::Declaration);
        }
        else
        {
            return this->vardecl();
        }
    }
    if (this->peek().kind == TokenKind::Function)
    {
        this->pop();
        Noderef var = this->make(this->consume(TokenKind::Identifier), NodeKind::Primary);
        bool is_method = false;
        while (this->peek().kind == TokenKind::Dot)
        {
            this->pop();
            Token token = this->consume(TokenKind::Identifier);
            var = this->make(var, this->make(token, NodeKind::Name), NodeKind::Property);
        }
        if (this->peek().kind == TokenKind::Colon)
        {
            is_method = true;
            this->pop();
            Token token = this->consume(TokenKind::Identifier);
            var = this->make(var, this->make(token, NodeKind::Name), NodeKind::Property);
        }
        Noderef body = this->function_body(is_method);
        return this->make(this->make(var, NodeKind::VarList), this->make(body, NodeKind::Explist), NodeKind::AssignStmt);
    }
    if (this->peek().kind == TokenKind::For)
    {
        this->pop();
        Token id = this->consume(TokenKind::Identifier);
        if (this->peek().kind == TokenKind::Equal)
        {
            return this->numeric_for_stmt(id);
        }
        else
        {
            return this->generic_for_stmt(id);
        }
    }
    return nullptr; // never reaches here
}

void Parser::error(string message, Token token)
{
    throw message + " (line " +
        std::to_string(token.line + 1) +
        ", at " +
        std::to_string(token.offset + 1) +
        ")";
}

bool is_end(BlockEnd end, Token token)
{
    if (end == BlockEnd::Eof)
    {
        return token.kind == TokenKind::Eof;
    }
    else if (end == BlockEnd::End)
    {
        return token.kind == TokenKind::End;
    }
    else if (end == BlockEnd::Else)
    {
        return token.kind == TokenKind::Else ||
               token.kind == TokenKind::ElseIf ||
               token.kind == TokenKind::End;
    }
    else if (end == BlockEnd::Until)
    {
        return token.kind == TokenKind::Until;
    }
    return false;
}

Noderef Parser::block(BlockEnd end)
{
    vector<Noderef> stmts;
    while (true)
    {
        if (is_end(end, this->peek()))
        {
            break;
        }
        if (this->peek().kind == TokenKind::Return)
        {
            this->pop();
            Noderef val = nullptr;
            if (!is_end(end, this->peek()))
                val = this->explist();
            while (this->peek().kind == TokenKind::Semicolon)
                this->pop();
            if (val)
                stmts.push_back(this->make(val, NodeKind::ReturnStmt));
            else
                stmts.push_back(this->make(NodeKind::ReturnStmt));
            break;
        }
        Noderef stmt = this->statement();
        if (stmt != nullptr)
        {
            stmts.push_back(stmt);
        }
    }
    return this->make(stmts, NodeKind::Block);
}

Noderef Parser::function_body(bool is_method)
{
    this->consume(TokenKind::LeftParen);
    vector<Noderef> parlist;
    bool ddd = false;
    if (this->peek().kind == TokenKind::Identifier)
    {
        parlist.push_back(this->make(this->make(this->pop(), NodeKind::Name), NodeKind::VarDecl));
    }
    else if (this->peek().kind == TokenKind::DotDotDot)
    {
        parlist.push_back(this->make(this->make(this->pop(), NodeKind::Name), NodeKind::VarDecl));
        ddd = true;
    }
    if (!ddd)
        while (this->peek().kind == TokenKind::Comma)
        {
            this->pop();
            if (this->peek().kind == TokenKind::DotDotDot)
            {
                parlist.push_back(this->make(this->pop(), NodeKind::Name));
                break;
            }
            parlist.push_back(this->make(this->make(this->consume(TokenKind::Identifier), NodeKind::Name), NodeKind::VarDecl));
        }
    this->consume(TokenKind::RightParen);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return this->make(this->make(parlist, NodeKind::NameList), block, is_method ? NodeKind::MethodBody : NodeKind::FunctionBody);
}
Noderef Parser::name_attrib()
{
    Token id = this->consume(TokenKind::Identifier);
    if (this->peek().kind == TokenKind::Less)
    {
        this->pop();
        Noderef name = this->make(id, NodeKind::Name);
        Noderef att = this->make(this->consume(TokenKind::Identifier), NodeKind::Name);
        this->consume(TokenKind::Greater);
        return this->make(name, att, NodeKind::VarDecl);
    }
    else
    {
        return this->make(this->make(id, NodeKind::Name), NodeKind::VarDecl);
    }
}

Noderef Parser::vardecl()
{
    vector<Noderef> vars;
    Noderef explist = nullptr;
    vars.push_back(this->name_attrib());
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        vars.push_back(this->name_attrib());
    }
    if (this->peek().kind == TokenKind::Equal)
    {
        this->pop();
        explist = this->explist();
    }
    if (explist)
        return this->make(this->make(vars, NodeKind::VarList), explist, NodeKind::Declaration);
    else
        return this->make(this->make(vars, NodeKind::VarList), NodeKind::Declaration);
}

Noderef Parser::fncall(Token op)
{
    if (op.kind == TokenKind::Literal)
        return this->make(this->make(op, NodeKind::Primary), NodeKind::Explist);
    else if (op.kind == TokenKind::LeftBrace)
        return this->make(this->table(), NodeKind::Explist);
    else // left parenthese
        return this->arglist();
}

Noderef Parser::expr_p(uint8_t pwr)
{
    Token t = this->pop();
    uint8_t prefix = check_prefix(t.kind);
    Noderef lhs = nullptr;
    if (prefix != 255)
    {
        Noderef rhs = this->expr_p(prefix);
        lhs = this->make(this->make(t, NodeKind::Operator), rhs, NodeKind::Unary);
    }
    else if (TOKEN_IS_PRIMARY(t.kind))
    {
        lhs = this->make(t, NodeKind::Primary);
    }
    else if (t.kind == TokenKind::LeftBrace)
    {
        lhs = this->table();
    }
    else if (t.kind == TokenKind::LeftParen)
    {
        lhs = this->expr();
        t = this->consume(TokenKind::RightParen);
    }
    else if (t.kind == TokenKind::Function)
    {
        lhs = this->function_body(false);
    }
    else
    {
        this->error("expression expected", t);
    }

    while (true)
    {
        Token op = this->peek();
        uint8_t p = check_postfix(op.kind);
        if (p != 255)
        {
            if (t.kind != TokenKind::Identifier && t.kind != TokenKind::RightParen)
                break;
            if (p < pwr)
                break;
            this->pop();
            if (op.kind == TokenKind::Dot)
            {
                Token field = this->consume(TokenKind::Identifier);
                lhs = this->make(lhs, this->make(field, NodeKind::Name), NodeKind::Property);
            }
            else if (op.kind == TokenKind::Colon)
            {
                Token fname = this->consume(TokenKind::Identifier);
                Noderef rhs = this->fncall(this->pop());
                lhs = this->make(lhs, this->make(fname, NodeKind::Name), rhs, NodeKind::MethodCall);
            }
            else if (op.kind == TokenKind::LeftBracket)
            {
                Noderef rhs = this->expr();
                this->consume(TokenKind::RightBracket);
                lhs = this->make(lhs, rhs, NodeKind::Index);
            }
            else
                lhs = this->make(lhs, this->fncall(op), NodeKind::Call);
        }
        else
        {
            if (!TOKEN_IS_BINARY(op.kind))
                break;
            uint16_t p = binpwr[op.kind % 256];
            uint8_t lp = p >> 8;
            uint8_t rp = p % 256;

            if (pwr > lp)
            {
                break;
            }
            this->pop();
            Noderef rhs = expr_p(rp);
            lhs = this->make(lhs, this->make(op, NodeKind::Operator), rhs, NodeKind::Binary);
        }
    }

    return lhs;
}

Token Parser::pop()
{
    Token t = this->current;
    if (t.kind == TokenKind::Error)
    {
        this->error(t.text(), t);
    }
    this->current = ahead;
    if (current.kind != TokenKind::Eof)
        this->ahead = this->lexer->next();
    return t;
}

Token Parser::peek()
{
    Token t = this->current;
    if (t.kind == TokenKind::Error)
    {
        this->error(t.text(), t);
    }
    return t;
}

Token Parser::look_ahead()
{
    Token t = this->ahead;
    if (t.kind == TokenKind::Error)
    {
        this->error(t.text(), t);
    }
    return t;
}

Ast Parser::parse()
{
    try
    {
        Ast tree = Ast(this->block(BlockEnd::Eof));
        if (this->peek().kind != TokenKind::Eof)
        {
            this->error("EOF expected", this->pop());
        }
        return tree;
    }
    catch (string message)
    {
        printf("lua: %s\n", message.c_str());
        return Ast(nullptr);
    }
}

Ast Parser::parse_exp()
{
    try
    {
        Ast tree = Ast(this->expr());
        if (this->peek().kind != TokenKind::Eof)
        {
            this->error("EOF expected", this->pop());
        }
        return tree;
    }
    catch (string message)
    {
        printf("lua: %s\n", message.c_str());
        return Ast(nullptr);
    }
}