#include <tap/tap.h>

void lexer_tests();
void parser_tests();
void compiler_tests();
void interpretor_tests();

int main()
{
    plan(NO_PLAN);

    lexer_tests();
    parser_tests();
    compiler_tests();
    interpretor_tests();

    done_testing();
    return 0;
}