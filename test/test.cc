#include "test.h"
#include <tap/tap.h>

void lexer_tests();
void parser_tests();
void compiler_tests();
void interpreter_tests();
void lua_tests();
void runtime_tests();

void test_assert(bool result, const char *message)
{
    ok(result, message);
}

int main()
{
    plan(NO_PLAN);

    lexer_tests();
    parser_tests();
    compiler_tests();
    interpreter_tests();
    runtime_tests();
    lua_tests();

    done_testing();
    return 0;
}