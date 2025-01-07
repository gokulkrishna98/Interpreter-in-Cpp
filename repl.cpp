#include "repl.h"
#include "lexer.h"
#include <string>
#include <iostream>

using namespace std;

namespace repl {
const string PROMPT = ">>>"; 
void start(){
    string input;
    while(true){
        cout << PROMPT;
        cin.ignore();
        getline(cin, input);
        lexer::Lexer l(input);
        lexer::Token tok = l.next_token();
        while(tok.type != lexer::TokenType::ENDOF){
            printf("%s\n", tok.val.c_str());
            tok = l.next_token();
        }
    }
}
} // namespace repl