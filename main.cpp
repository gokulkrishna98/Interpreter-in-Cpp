// #include "lexer.h"
#include "parser.h"
// #include "repl.h"

using namespace std;

int main(int argc, char** argv){
    printf("interp running\n");
    // lexer::test_next_token();
    // repl::start();
    parser::test_let_statements();
    return 0;
}