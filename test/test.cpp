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
                {
                    return NULL;
                }
                if (kidx == kinds.size())
                {
                    return NULL;
                }
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
            {
                offset++;
            }
            orig[ptr] = c;
            ptr++;
        }
    }

    if (is_token)
    {
        return NULL;
    }
    if (kidx < kinds.size())
    {
        return NULL;
    }

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
    free(orig);
    return rsl && tkns.size() == tidx;
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

    const char *prefix = "lexer : ";
    size_t plen = strlen(prefix);
    size_t mlen = strlen(message);
    char *mes = (char *)malloc(mlen + plen + 1);
    strcpy(mes, prefix);
    strcpy(mes + plen, message);
    mes[mlen + plen] = '\0';
    ok(lexer_test(text, kinds), mes);
    free(mes);
}

int main()
{
    plan(NO_PLAN);
    lxtest("empty string", "");
    lxtest("space", "    ");
    lxtest("space/endl", " \n   \n   ");
    lxtest("single char (+)", "$+$", Plus);
    lxtest("single char after space (})", "    $}$", RightBrace);
    lxtest("space after single char (^)", "$^$ \n  ", Power);
    done_testing();
    return 0;
}