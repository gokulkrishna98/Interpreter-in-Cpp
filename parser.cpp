#include "parser.h"

namespace parser {

string LetStatement::token_literal(){
    return let_token.val;
}

string Identifier::token_literal(){
    return token.val;
}

bool test_let_statement(Statement* s, string id){
    if(s->token_literal() != "let"){
        printf("expected to get `let` but got %s\n", s->token_literal().c_str());
        return false;
    }

    auto let_statment = dynamic_cast<LetStatement*>(s);
    if(let_statment == nullptr){
        printf("could not cast statment to let_statement\n");
        return false;
    }

    if(let_statment->name->value != id){
        printf("letstatement id is different, expected %s, got %s\n", 
            id.c_str(),
            let_statment->name->value.c_str());
        return false;
    }
    return true;
}

void test_let_statements(){
    string input = R"(
let x = 5;
let y = 10;
let foobar = 838383;
    )";

    auto l = lexer::Lexer(input);
    auto p = Parser(l);

    auto program = p.parse_program();

    if(program == nullptr){
        printf("parse program returned nullptr\n");
        return;
    }

    if(program->statements.size() != 3){
        printf("program does not contain 3 statements\n");
        return;
    }

    vector<string> expected_id = {"x", "y", "foobar"};

    for(int i=0; i<expected_id.size(); i++){
        auto stmt= program->statements[i].get();
        if(!test_let_statement(stmt, expected_id[i])){
            return;
        }
    }
}


}