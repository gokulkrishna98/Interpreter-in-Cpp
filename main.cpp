// #include "lexer.h"
#include "parser.h"
// #include "repl.h"

using namespace std;

int main(int argc, char** argv){
    // printf("interp running\n");
    // lexer::test_next_token();
    // repl::start();
    // parser::test_let_statements();
    // parser::test_ret_statements();
    // parser::test_string();
    // parser::test_identifier_expression();
    // parser::test_integer_literal_expression();
    // parser::test_parsing_prefix_expression();
    // parser::test_parsing_infix_expression();
    parser::test_operator_precedence_parsing();
    return 0;
}