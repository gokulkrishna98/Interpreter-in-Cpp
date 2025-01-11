#include "parser.h"
#include <cstdio>
#include <memory>

namespace parser {

string LetStatement::token_literal() const {
    return let_token.val;
}

string Identifier::token_literal() const {
    return token.val;
}

bool Parser::_cur_tok_is(lexer::TokenType t) {
    return cur_token.type == t;
}

bool Parser::_peek_tok_is(lexer::TokenType t){
    return peek_token.type == t;
}

bool Parser::_expect_peek(lexer::TokenType t){
    if(_peek_tok_is(t)){
        next_token();
        return true;
    }else{
        return false;
    }
}

unique_ptr<LetStatement> Parser::parse_let_statement(){
    auto stmt = make_unique<LetStatement>();  
    stmt->let_token = cur_token;

    if(!_expect_peek(lexer::TokenType::ID)){
        return nullptr;
    }

    stmt->name = make_unique<Identifier>(cur_token, cur_token.val);
    if(!_expect_peek(lexer::TokenType::ASSIGN)){
        return nullptr;
    }

    while(!_cur_tok_is(lexer::TokenType::SEMICOLON)){
        next_token();
    }

    return stmt;
}

unique_ptr<Statement> Parser::parse_statement(){
    switch(cur_token.type){
        case lexer::TokenType::LET: return parse_let_statement();
        default: return nullptr;
    }
}

unique_ptr<Program> Parser::parse_program(){
    auto program = std::make_unique<Program>();
    while(cur_token.type != lexer::TokenType::ENDOF){
        auto stmt = parse_statement();
        if(stmt != nullptr){
            program->statements.push_back(std::move(stmt));
        }
        next_token();
    }
    return program;
}

bool test_let_statement(const Statement* s, string id){
    if(s->token_literal() != "let"){
        printf("expected to get `let` but got %s\n", s->token_literal().c_str());
        return false;
    }

    auto let_statment = dynamic_cast<const LetStatement*>(s);
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
        printf("program does not contain 3 statements: found %ld statements\n",
            program->statements.size());
        return;
    }

    vector<string> expected_id = {"x", "y", "foobar"};

    for(int i=0; i<expected_id.size(); i++){
        const Statement* stmt= program->statements[i].get();
        if(!test_let_statement(stmt, expected_id[i])){
            return;
        }
    }
    printf("[3/3] all test cases passed !!");
}


}