#include "lexer.h"
#include "repl.h"

using namespace std;

int main(int argc, char** argv){
    printf("interp running\n");
    // lexer::test_next_token();
    repl::start();
    return 0;
}