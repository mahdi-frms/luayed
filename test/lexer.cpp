#include <tap/tap.h>
#include <lexer.hpp>
#include <cstring>
#include <stdarg.h>

char *tokenize_test(const char *text, vector<TokenKind> &kinds, vector<Token> &tokens)
{
    const char marker = '$';
    size_t textlen = strlen(text);
    char *orig = new char[textlen + 1];
    int is_token = 0;
    size_t kidx = 0;
    size_t line = 0, offset = 0, ptr = 0;
    size_t cur_line = 0, cur_offset = 0, cur_ptr = 0;
    for (size_t i = 0; i < textlen; i++)
    {
        char c = text[i];
        if (is_token == 2)
        {
            is_token--;
            cur_ptr = ptr;
            cur_line = line;
            cur_offset = offset;
        }
        if (is_token == 1)
        {
            if (c == marker)
            {
                if (ptr == cur_ptr)
                    return NULL;
                if (kidx == kinds.size())
                    return NULL;
                else
                {
                    is_token = 0;
                    tokens.push_back(Token(orig + cur_ptr, ptr - cur_ptr, cur_line, cur_offset, kinds[kidx++]));
                }
            }
        }
        else
        {
            if (c == marker)
                is_token = 2;
        }

        if (c != marker)
        {
            if (c == '\n')
            {
                offset = 0;
                line++;
            }
            else
                offset++;
            orig[ptr] = c;
            ptr++;
        }
    }
    if (is_token)
        return NULL;
    if (kidx < kinds.size())
        return NULL;
    orig[ptr] = '\0';
    return orig;
}

bool tcmp(Token &t1, Token &t2)
{
    return t1.kind == t2.kind &&
           t1.len == t2.len &&
           t1.line == t2.line &&
           t1.offset == t2.offset &&
           t1.str == t2.str;
}

bool lexer_test(const char *text, vector<TokenKind> kinds)
{
    vector<Token> tkns;
    char *orig = tokenize_test(text, kinds, tkns);
    if (orig == NULL)
    {
        fprintf(stderr, "LEXER TEST CRASH!\n");
        exit(1);
    }
    Lexer lxr = Lexer(orig);
    size_t tidx = 0;
    bool rsl = true;
    while (true)
    {
        Token t = lxr.next();
        if (t.kind == TokenKind::Eof)
        {
            break;
        }
        if (tidx == tkns.size())
        {
            rsl = false;
            break;
        }
        if (!tcmp(tkns[tidx++], t))
        {
            rsl = false;
            break;
        }
    }
    delete[] orig;
    return rsl && tkns.size() == tidx;
}

char *concat(const char *s1, const char *s2)
{
    size_t plen = strlen(s1);
    size_t mlen = strlen(s2);
    char *mes = (char *)malloc(mlen + plen + 1);
    strcpy(mes, s1);
    strcpy(mes + plen, s2);
    mes[mlen + plen] = '\0';
    return mes;
}

void lxerrr(const char *message, const char *text)
{
    bool rsl = false;
    Lexer lxr = Lexer(text);
    while (true)
    {
        Token t = lxr.next();
        rsl = t.kind == TokenKind::Error;
        if (rsl || t.kind == TokenKind::Eof)
        {
            break;
        }
    }
    char *mes = concat("lexer : ", message);
    ok(rsl, mes);
    free(mes);
}

void lxtest(const char *message, const char *text, ...)
{
    vector<TokenKind> kinds;
    va_list list;
    va_start(list, text);
    int count = 0;
    for (const char *ptr = text; *ptr != 0; ptr++)
    {
        if (*ptr == '$')
            count++;
    }
    count /= 2;
    while (count--)
    {
        kinds.push_back((TokenKind)va_arg(list, int));
    }
    va_end(list);

    char *mes = concat("lexer : ", message);
    ok(lexer_test(text, kinds), mes);
    free(mes);
}

void lexer_tests()
{
    lxtest("empty string", "");
    lxtest("space", "    ");
    lxtest("space & endl", " \n   \n   ");

    lxtest("single char (+)", "$+$", Plus);
    lxtest("single char after space (})", "    $}$", RightBrace);
    lxtest("space after single char (^)", "$^$ \n  ", Power);
    lxtest("multiple single chars", "$;$$*$$&$$)$$($", Semicolon, Multiply, BinAnd, RightParen, LeftParen);
    lxtest("multiple singles with space in between", "  \n   $%$ $+$\n$+$ $,$$,$", Modulo, Plus, Plus, Comma, Comma);
    lxtest("dots", "$.$ $..$ $...$", Dot, DotDot, DotDotDot);
    lxtest("division", "$/$ $//$", FloatDivision, FloorDivision);
    lxtest("less & greater", "$<$ $<=$ $<<$ \n $>$ $>=$ $>>$", Less, LessEqual, LeftShift, Greater, GreaterEqual, RightShift);
    lxtest("equal & colon", "$=$ $==$ $:$ $::$", Equal, EqualEqual, Colon, ColonColon);
    lxtest("negate", "$~$ $~=$", Negate, NotEqual);
    lxtest("minus", "$-$", Minus);
    lxerrr("invalid char", "@");

    lxtest("comment", "$+$ -- .", Plus);
    lxtest("single line comment", "-- .\n$.$", Dot);
    lxtest("multiline comment", "$+$--[[\n .-- \n]]$.$", Plus, Dot);
    lxtest("multilevel comment", "$&$--[==[ + ] %]==]$)$", BinAnd, RightParen);
    lxtest("exec path", "#!/usr/bin/lua");

    lxtest("identifier", "$foo$", Identifier);
    lxtest("multiple ids", "  $foo$ \n$bar$ ", Identifier, Identifier);
    lxtest("ids with underscore", "  $__foo_$ \n$bar_$ ", Identifier, Identifier);
    lxtest("ids with digits", " $f1223$ $num2uy4$ $_t_3454_5t3$  ", Identifier, Identifier, Identifier);

    lxtest("keyword (and)", "$and$", And);
    lxtest("keyword (break)", "$break$", Break);
    lxtest("keyword (do)", "$do$", Do);
    lxtest("keyword (end)", "$end$", End);
    lxtest("keyword (else)", "$else$", Else);
    lxtest("keyword (elseif)", "$elseif$", ElseIf);
    lxtest("keyword (for)", "$for$", For);
    lxtest("keyword (false)", "$false$", False);
    lxtest("keyword (function)", "$function$", Function);
    lxtest("keyword (goto)", "$goto$", Goto);
    lxtest("keyword (in)", "$in$", In);
    lxtest("keyword (if)", "$if$", If);
    lxtest("keyword (local)", "$local$", Local);
    lxtest("keyword (not)", "$not$", Not);
    lxtest("keyword (nil)", "$nil$", Nil);
    lxtest("keyword (or)", "$or$", Or);
    lxtest("keyword (return)", "$return$", Return);
    lxtest("keyword (repeat)", "$repeat$", Repeat);
    lxtest("keyword (then)", "$then$", Then);
    lxtest("keyword (true)", "$true$", True);
    lxtest("keyword (until)", "$until$", Until);
    lxtest("keyword (while)", "$while$", While);

    lxtest("single digit", "$0$", Number);
    lxtest("single digit", "$0$$+$", Number, Plus);
    lxerrr("single digit .. letter", "0t");
    lxtest("Integer", "$179$", Number);
    lxerrr("Integer with letter in between", "179r76");
    lxtest("float", "$179.45$", Number);
    lxerrr("float multi-precision", "1.794.5");
    lxtest("float without int", "$.5$", Number);
    lxtest("float without decimal", "$7.$", Number);
    lxtest("Integer with exponent", "$3e7$ $0e9$ $4e-7$", Number, Number, Number);
    lxtest("float with exponent", "$.3e7$ $0.4e-11$", Number, Number);
    lxtest("float without decimal with exponent", "$3.e-6$", Number);
    lxerrr("exponent without digits", ".2e");
    lxtest("hex", "$0x3$", Number);
    lxerrr("hex without digits", "0x");
    lxerrr("hex followed by dot", "0x34.");

    lxtest("empty literal", "$''$ $\"\"$", Literal, Literal);
    lxtest("basic literal", "$'text'$", Literal);
    lxerrr("unclosed literal", "'");
    lxerrr("closed literal after endline", "'\n'");
    lxtest("escapes", "$'\\a\\b\\t\\n\\v\\r\\f \\\\ \\' \\\" \\\n \\[ \\] '$", Literal);
    lxerrr("invalid escape", "'\\s'");
    lxtest("escape \\\"", "$\"\\\"\"$", Literal);
    lxtest("escape \\z", "$'\\z\n\n\n\nstr'$", Literal);
    lxtest("escape byte", "$\" \\234 \"$", Literal);
    lxerrr("invalid byte", "'\\256'");
    lxtest("escape hex", "$'\\x4e'$", Literal);
    lxerrr("invalid hex", "'\\x4t'");
    lxtest("multiline literal", "$[[]]$ $[[  \n  str ]]$", Literal, Literal);
    lxtest("multilevel literal", "$[===[ ]==] ]===]$", Literal);
    lxerrr("unclosed multilevel literal", "[===[ ]==]");
}