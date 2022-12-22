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

Parser::Parser(ILexer *lexer) : lexer(lexer), current(token_none())
{
    this->current = this->lexer->next();
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
    return this->expr(token_none());
}

Noderef Parser::expr(Token t)
{
    return this->expr_p(0, t);
}

Token Parser::consume(TokenKind kind)
{
    Token t = this->pop();
    if (t.kind != kind)
    {
        this->error(error_expected_token(kind), t);
    }
    return t;
}

Noderef Parser::id_field(Token t)
{
    Noderef id = Ast::make(t, NodeKind::Name);
    this->consume(TokenKind::Equal);
    return Ast::make(id, this->expr(), NodeKind::IdField);
}

Noderef Parser::expr_field()
{
    this->pop(); // [
    Noderef field = this->expr();
    this->consume(TokenKind::RightBracket);
    this->consume(TokenKind::Equal);
    return Ast::make(field, this->expr(), NodeKind::ExprField);
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
            Token n = this->pop();
            if (this->peek().kind == TokenKind::Equal)
            {
                items.push_back(this->id_field(n));
            }
            else
            {
                items.push_back(this->expr(n));
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
            t.kind != TokenKind::RightBrace)
        {
            this->error(error_expected_token(TokenKind::RightBrace), t);
        }
        else if (t.kind != TokenKind::RightBrace)
        {
            this->pop();
        }
    }
    return Ast::make(items, NodeKind::Table);
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
    return Ast::make(args, NodeKind::Explist);
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
            this->error(error_expected_variable(), this->peek());
        items.push_back(var);
    }
    return Ast::make(items, NodeKind::Explist);
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
    return Ast::make(items, NodeKind::Explist);
}

Noderef Parser::generic_for_stmt(Token identifier)
{
    vector<Noderef> namelist;
    namelist.push_back(Ast::make(Ast::make(identifier, NodeKind::Name), NodeKind::VarDecl));
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        namelist.push_back(Ast::make(Ast::make(this->consume(TokenKind::Identifier), NodeKind::Name), NodeKind::VarDecl));
    }
    this->consume(TokenKind::In);
    Noderef explist = this->explist();
    this->consume(TokenKind::Do);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return Ast::make(Ast::make(namelist, NodeKind::NameList), explist, block, NodeKind::GenericFor);
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
        return Ast::make(Ast::make(Ast::make(identifier, NodeKind::Name), NodeKind::VarDecl), from, to, step, block, NodeKind::NumericFor);
    else
        return Ast::make(Ast::make(Ast::make(identifier, NodeKind::Name), NodeKind::VarDecl), from, to, block, NodeKind::NumericFor);
}

Noderef Parser::while_stmt()
{
    this->pop();
    Noderef expr = this->expr();
    this->consume(TokenKind::Do);
    Noderef blk = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return Ast::make(expr, blk, NodeKind::WhileStmt);
}
Noderef Parser::repeat_stmt()
{
    this->pop();
    Noderef blk = this->block(BlockEnd::Until);
    this->consume(TokenKind::Until);
    Noderef expr = this->expr();
    return Ast::make(blk, expr, NodeKind::RepeatStmt);
}
Noderef Parser::if_stmt()
{
    vector<Noderef> clauses;
    this->pop();
    Noderef expr = this->expr();
    this->consume(TokenKind::Then);
    Noderef block = this->block(BlockEnd::Else);
    clauses.push_back(Ast::make(expr, block, NodeKind::IfClause));
    while (this->peek().kind == TokenKind::ElseIf)
    {
        this->pop();
        expr = this->expr();
        this->consume(TokenKind::Then);
        block = this->block(BlockEnd::Else);
        clauses.push_back(Ast::make(expr, block, NodeKind::ElseIfClause));
    }
    if (this->peek().kind == TokenKind::Else)
    {
        this->pop();
        clauses.push_back(Ast::make(this->block(BlockEnd::End), NodeKind::ElseClause));
    }
    this->consume(TokenKind::End);
    return Ast::make(clauses, NodeKind::IfStmt);
}

Noderef Parser::statement()
{
    if (!TOKEN_IS_STATEMENT(this->peek().kind))
    {
        Noderef s = this->expr();
        if (s->get_kind() == NodeKind::Call || s->get_kind() == NodeKind::MethodCall)
            return Ast::make(s, NodeKind::CallStmt);

        else if (is_var(s))
        {
            Noderef vars = this->varlist(s);
            this->consume(TokenKind::Equal);
            Noderef vals = this->explist();
            return Ast::make(vars, vals, NodeKind::AssignStmt);
        }
        else
        {
            this->error(error_expected_variable(), this->peek());
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
        return Ast::make(id, NodeKind::LabelStmt);
    }
    if (this->peek().kind == TokenKind::Break)
    {
        this->pop();
        return Ast::make(NodeKind::BreakStmt);
    }
    if (this->peek().kind == TokenKind::Goto)
    {
        this->pop();
        return Ast::make(this->consume(TokenKind::Identifier), NodeKind::GotoStmt);
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

            return Ast::make(
                Ast::make(
                    Ast::make(
                        Ast::make(t, NodeKind::Name), NodeKind::VarDecl),
                    NodeKind::VarList),
                Ast::make(this->function_body(false), NodeKind::Explist), NodeKind::Declaration);
        }
        else
        {
            return this->vardecl();
        }
    }
    if (this->peek().kind == TokenKind::Function)
    {
        this->pop();
        Noderef var = Ast::make(this->consume(TokenKind::Identifier), NodeKind::Primary);
        bool is_method = false;
        while (this->peek().kind == TokenKind::Dot)
        {
            this->pop();
            Token token = this->consume(TokenKind::Identifier);
            var = Ast::make(var, Ast::make(token, NodeKind::Name), NodeKind::Property);
        }
        if (this->peek().kind == TokenKind::Colon)
        {
            is_method = true;
            this->pop();
            Token token = this->consume(TokenKind::Identifier);
            var = Ast::make(var, Ast::make(token, NodeKind::Name), NodeKind::Property);
        }
        Noderef body = this->function_body(is_method);
        return Ast::make(Ast::make(var, NodeKind::VarList), Ast::make(body, NodeKind::Explist), NodeKind::AssignStmt);
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

void Parser::error(LError err, Token token)
{
    err.line = token.line;
    err.offset = token.offset;
    this->err = err;
    throw 1;
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
                stmts.push_back(Ast::make(val, NodeKind::ReturnStmt));
            else
                stmts.push_back(Ast::make(NodeKind::ReturnStmt));
            break;
        }
        Noderef stmt = this->statement();
        if (stmt != nullptr)
        {
            stmts.push_back(stmt);
        }
    }
    return Ast::make(stmts, NodeKind::Block);
}

Noderef Parser::function_body(bool is_method)
{
    this->consume(TokenKind::LeftParen);
    vector<Noderef> parlist;
    bool ddd = false;
    if (this->peek().kind == TokenKind::Identifier)
    {
        parlist.push_back(Ast::make(Ast::make(this->pop(), NodeKind::Name), NodeKind::VarDecl));
    }
    else if (this->peek().kind == TokenKind::DotDotDot)
    {
        parlist.push_back(Ast::make(Ast::make(this->pop(), NodeKind::Name), NodeKind::VarDecl));
        ddd = true;
    }
    if (!ddd)
        while (this->peek().kind == TokenKind::Comma)
        {
            this->pop();
            if (this->peek().kind == TokenKind::DotDotDot)
            {
                parlist.push_back(Ast::make(Ast::make(this->pop(), NodeKind::Name), NodeKind::VarDecl));
                break;
            }
            parlist.push_back(Ast::make(Ast::make(this->consume(TokenKind::Identifier), NodeKind::Name), NodeKind::VarDecl));
        }
    this->consume(TokenKind::RightParen);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return Ast::make(Ast::make(parlist, NodeKind::NameList), block, is_method ? NodeKind::MethodBody : NodeKind::FunctionBody);
}
Noderef Parser::name_attrib()
{
    Token id = this->consume(TokenKind::Identifier);
    if (this->peek().kind == TokenKind::Less)
    {
        this->pop();
        Noderef name = Ast::make(id, NodeKind::Name);
        Noderef att = Ast::make(this->consume(TokenKind::Identifier), NodeKind::Name);
        this->consume(TokenKind::Greater);
        return Ast::make(name, att, NodeKind::VarDecl);
    }
    else
    {
        return Ast::make(Ast::make(id, NodeKind::Name), NodeKind::VarDecl);
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
        return Ast::make(Ast::make(vars, NodeKind::VarList), explist, NodeKind::Declaration);
    else
        return Ast::make(Ast::make(vars, NodeKind::VarList), NodeKind::Declaration);
}

Noderef Parser::fncall(Token op)
{
    if (op.kind == TokenKind::Literal)
        return Ast::make(Ast::make(op, NodeKind::Primary), NodeKind::Explist);
    else if (op.kind == TokenKind::LeftBrace)
        return Ast::make(this->table(), NodeKind::Explist);
    else // left parenthese
        return this->arglist();
}

Noderef Parser::expr_p(uint8_t pwr, Token tt)
{
    Token t = tt.kind == TokenKind::None ? this->pop() : tt;
    uint8_t prefix = check_prefix(t.kind);
    Noderef lhs = nullptr;
    if (prefix != 255)
    {
        Noderef rhs = this->expr_p(prefix, token_none());
        lhs = Ast::make(Ast::make(t, NodeKind::Operator), rhs, NodeKind::Unary);
    }
    else if (TOKEN_IS_PRIMARY(t.kind))
    {
        lhs = Ast::make(t, NodeKind::Primary);
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
        this->error(error_expected_expression(), t);
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
                lhs = Ast::make(lhs, Ast::make(field, NodeKind::Name), NodeKind::Property);
            }
            else if (op.kind == TokenKind::Colon)
            {
                Token fname = this->consume(TokenKind::Identifier);
                Noderef rhs = this->fncall(this->pop());
                lhs = Ast::make(lhs, Ast::make(fname, NodeKind::Name), rhs, NodeKind::MethodCall);
            }
            else if (op.kind == TokenKind::LeftBracket)
            {
                Noderef rhs = this->expr();
                this->consume(TokenKind::RightBracket);
                lhs = Ast::make(lhs, rhs, NodeKind::Index);
            }
            else
                lhs = Ast::make(lhs, this->fncall(op), NodeKind::Call);
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
            Noderef rhs = expr_p(rp, token_none());
            lhs = Ast::make(lhs, Ast::make(op, NodeKind::Operator), rhs, NodeKind::Binary);
        }
    }

    return lhs;
}

Token Parser::pop()
{
    Token t = this->current;
    if (t.kind == TokenKind::Error)
    {
        this->error(this->lexer->get_error(), t);
    }
    this->current = this->lexer->next();
    return t;
}

Token Parser::peek()
{
    Token t = this->current;
    if (t.kind == TokenKind::Error)
    {
        this->error(this->lexer->get_error(), t);
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
            this->error(error_expected_token(TokenKind::Eof), this->pop());
        }
        return tree;
    }
    catch (int _)
    {
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
            this->error(error_expected_token(TokenKind::Eof), this->pop());
        }
        return tree;
    }
    catch (int _)
    {
        return Ast(nullptr);
    }
}

LError Parser::get_error()
{
    return this->err;
}
