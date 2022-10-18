#include <tap/tap.h>

void lexer_tests();

int main()
{
    plan(NO_PLAN);

    lexer_tests();

    done_testing();
    return 0;
}