#include "parser.hpp"
#include <utility>
using namespace ast;

uint16_t priorities(uint8_t l, uint8_t r)
{
    return l * 256 + r;
}

uint8_t check_prefix(TokenKind kind)
{
    if (kind == TokenKind::Not || kind == TokenKind::Negate || kind == TokenKind::Minus || kind == TokenKind::Length)
        return 31;
    return 255;
}

Token errtoken()
{
    return Token("", 0, 0, TokenKind::Error);
}

Token token_none()
{
    return Token("", 0, 0, TokenKind::None);
}

bool is_primary(TokenKind kind)
{
    TokenKind accept[] = {
        TokenKind::Nil,
        TokenKind::True,
        TokenKind::False,
        TokenKind::Number,
        TokenKind::Literal,
        TokenKind::DotDotDot,
        TokenKind::Identifier,
    };
    for (int i = 0; i < sizeof(accept) / sizeof(TokenKind); i++)
    {
        if (accept[i] == kind)
        {
            return true;
        }
    }
    return false;
}

bool is_var(Noderef node)
{
    if (node->get_kind() == NodeKind::Index || node->get_kind() == NodeKind::Property)
        return true;
    if (node->get_kind() != NodeKind::Primary)
        return false;
    return node->as<Primary>().token.kind == TokenKind::Identifier;
}

uint8_t check_postfix(TokenKind kind)
{
    if (kind == TokenKind::LeftBrace ||
        kind == TokenKind::LeftParen ||
        kind == TokenKind::LeftBracket ||
        kind == TokenKind::Dot ||
        kind == TokenKind::Colon ||
        kind == TokenKind::Literal)
    {
        return 35;
    }
    return 255;
}

uint16_t check_binary(TokenKind kind)
{
    if (kind == TokenKind::Or)
        return priorities(11, 12);
    if (kind == TokenKind::And)
        return priorities(13, 14);
    if (kind == TokenKind::EqualEqual ||
        kind == TokenKind::NotEqual ||
        kind == TokenKind::Greater ||
        kind == TokenKind::Less ||
        kind == TokenKind::GreaterEqual ||
        kind == TokenKind::LessEqual)
        return priorities(15, 16);
    if (kind == TokenKind::BinOr)
        return priorities(17, 18);
    if (kind == TokenKind::Negate)
        return priorities(19, 20);
    if (kind == TokenKind::BinAnd)
        return priorities(21, 22);
    if (kind == TokenKind::RightShift || kind == TokenKind::LeftShift)
        return priorities(23, 24);
    if (kind == TokenKind::DotDot)
        return priorities(26, 25);
    if (kind == TokenKind::Plus || kind == TokenKind::Minus)
        return priorities(27, 28);
    if (kind == TokenKind::Multiply ||
        kind == TokenKind::FloatDivision ||
        kind == TokenKind::FloorDivision ||
        kind == TokenKind::Modulo)
        return priorities(29, 30);
    if (kind == TokenKind::Power)
        return priorities(34, 33);
    return (uint16_t)-1;
}

Parser::Parser(Lexer &lexer) : lexer(lexer)
{
    this->tokens = this->lexer.drain();
}

Node::Node(Gnode inner, NodeKind kind) : inner(inner), kind(kind)
{
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

Noderef Parser::id_field()
{
    Token id = this->pop();
    this->consume(TokenKind::Equal);
    return make_id_field(id, this->expr());
}

Noderef Parser::expr_field()
{
    this->pop(); // [
    Noderef field = this->expr();
    this->consume(TokenKind::RightBracket);
    this->consume(TokenKind::Equal);
    return make_expr_field(field, this->expr());
}

Token Parser::ahead()
{
    Token t = this->tokens[1];
    if (t.kind == TokenKind::Error)
    {
        throw t.text;
    }
    return t;
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
            if (this->ahead().kind == TokenKind::Equal)
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
    return make_table(std::move(items));
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
    return make_explist(args);
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
    return make_explist(items);
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
    return make_explist(items);
}

Noderef Parser::generic_for_stmt(Token identifier)
{
    vector<Token> namelist;
    namelist.push_back(identifier);
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        namelist.push_back(this->consume(TokenKind::Identifier));
    }
    this->consume(TokenKind::In);
    Noderef explist = this->explist();
    this->consume(TokenKind::Do);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return make_generic_for_stmt(std::move(namelist), explist, block);
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
    return make_numeric_for_stmt(identifier, from, to, step, block);
}

Noderef Parser::while_stmt()
{
    this->pop();
    Noderef expr = this->expr();
    this->consume(TokenKind::Do);
    Noderef blk = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return make_white_stmt(expr, blk);
}
Noderef Parser::repeat_stmt()
{
    this->pop();
    Noderef blk = this->block(BlockEnd::Until);
    this->consume(TokenKind::Until);
    Noderef expr = this->expr();
    return make_repeat_stmt(expr, blk);
}
Noderef Parser::if_stmt()
{
    vector<Noderef> exprs;
    vector<Noderef> blocks;
    this->pop();
    exprs.push_back(this->expr());
    this->consume(TokenKind::Then);
    blocks.push_back(this->block(BlockEnd::Else));
    while (this->peek().kind == TokenKind::ElseIf)
    {
        this->pop();
        exprs.push_back(this->expr());
        this->consume(TokenKind::Then);
        blocks.push_back(this->block(BlockEnd::Else));
    }
    if (this->peek().kind == TokenKind::Else)
    {
        this->pop();
        exprs.push_back(nullptr);
        blocks.push_back(this->block(BlockEnd::End));
    }
    this->consume(TokenKind::End);
    return make_if_stmt(exprs, blocks);
}

Noderef Parser::statement()
{
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
        return make_label_stmt(id);
    }
    if (this->peek().kind == TokenKind::Break)
    {
        this->pop();
        return make_break_stmt();
    }
    if (this->peek().kind == TokenKind::Goto)
    {
        this->pop();
        return make_goto_stmt(this->consume(TokenKind::Identifier));
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
            return make_declaration({t}, {token_none()}, this->function_body());
        }
        else
        {
            return this->vardecl();
        }
    }
    if (this->peek().kind == TokenKind::Function)
    {
        this->pop();
        Noderef name = make_primary(this->consume(TokenKind::Identifier));
        Token identifier = token_none();
        while (this->peek().kind == TokenKind::Dot)
        {
            this->pop();
            name = make_property(name, this->consume(TokenKind::Identifier));
        }
        if (this->peek().kind == TokenKind::Colon)
        {
            this->pop();
            identifier = this->consume(TokenKind::Identifier);
            name = make_property(name, identifier);
        }
        Noderef body = this->function_body();
        if (identifier.kind == TokenKind::Identifier)
        {
            FunctionBody &fn = body->as<FunctionBody>();
            fn.parlist.insert(fn.parlist.begin(), Token("self", 0, 0, TokenKind::Identifier));
        }
        return make_assign_stmt({name}, {body});
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
    Noderef s = this->expr();
    if (s->get_kind() == NodeKind::Call || s->get_kind() == NodeKind::MethodCall)
        return make_call_stmt(s);

    else if (is_var(s))
    {
        Noderef vars = this->varlist(s);
        this->consume(TokenKind::Equal);
        Noderef vals = this->explist();
        return make_assign_stmt(vars, vals);
    }
    else
    {
        this->error("unexpected expression", this->peek());
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
            stmts.push_back(make_return_stmt(val));
            break;
        }
        Noderef stmt = this->statement();
        if (stmt != nullptr)
        {
            stmts.push_back(stmt);
        }
    }
    return make_block(stmts);
}

Noderef Parser::function_body()
{
    this->consume(TokenKind::LeftParen);
    vector<Token> parlist;
    bool ddd = false;
    if (this->peek().kind == TokenKind::Identifier)
    {
        parlist.push_back(this->pop());
    }
    else if (this->peek().kind == TokenKind::DotDotDot)
    {
        parlist.push_back(this->pop());
        ddd = true;
    }
    if (!ddd)
        while (this->peek().kind == TokenKind::Comma)
        {
            this->pop();
            if (this->peek().kind == TokenKind::DotDotDot)
            {
                parlist.push_back(this->pop());
                break;
            }
            parlist.push_back(this->consume(TokenKind::Identifier));
        }
    this->consume(TokenKind::RightParen);
    Noderef block = this->block(BlockEnd::End);
    this->consume(TokenKind::End);
    return make_function_body(std::move(parlist), block);
}
Token Parser::name_attrib(Token *attrib)
{
    *attrib = token_none();
    Token id = this->consume(TokenKind::Identifier);
    if (this->peek().kind == TokenKind::Less)
    {
        this->pop();
        *attrib = this->consume(TokenKind::Identifier);
        this->consume(TokenKind::Greater);
    }
    return id;
}

Noderef Parser::vardecl()
{
    vector<Token> names;
    vector<Token> attribs;
    Noderef explist = nullptr;
    Token att = token_none();
    names.push_back(this->name_attrib(&att));
    attribs.push_back(att);
    while (this->peek().kind == TokenKind::Comma)
    {
        this->pop();
        names.push_back(this->name_attrib(&att));
        attribs.push_back(att);
    }
    if (this->peek().kind == TokenKind::Equal)
    {
        this->pop();
        explist = this->explist();
    }
    return make_declaration(std::move(names), std::move(attribs), explist);
}

Noderef Parser::fncall(Token op)
{
    if (op.kind == TokenKind::Literal)
        return make_primary(op);
    else if (op.kind == TokenKind::LeftBrace)
        return this->table();
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
        lhs = make_unary(rhs, t);
    }
    else if (is_primary(t.kind))
    {
        lhs = make_primary(t);
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
        lhs = this->function_body();
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
                lhs = make_property(lhs, field);
            }
            else if (op.kind == TokenKind::Colon)
            {
                Token fname = this->consume(TokenKind::Identifier);
                Noderef rhs = this->fncall(this->pop());
                lhs = make_method_call(lhs, fname, rhs);
            }
            else if (op.kind == TokenKind::LeftBracket)
            {
                Noderef rhs = this->expr();
                this->consume(TokenKind::RightBracket);
                lhs = make_index(lhs, rhs);
            }
            else
                lhs = make_call(lhs, this->fncall(op));
        }
        else
        {
            uint16_t p = check_binary(op.kind);
            if (p == (uint16_t)-1)
                break;
            uint8_t lp = p >> 8;
            uint8_t rp = p % 256;

            if (pwr > lp)
            {
                break;
            }
            this->pop();
            Noderef rhs = expr_p(rp);
            lhs = make_binary(lhs, rhs, op);
        }
    }

    return lhs;
}

Token Parser::pop()
{
    Token t = this->tokens.front();
    if (t.kind == TokenKind::Eof)
    {
        error("PARSER CRASH: illegal consumption of EOF", t);
    }
    this->tokens.erase(this->tokens.begin());
    if (t.kind == TokenKind::Error)
    {
        this->error(t.text, t);
    }
    return t;
}

Token Parser::peek()
{
    Token t = this->tokens.front();
    if (t.kind == TokenKind::Error)
    {
        this->error(t.text, t);
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